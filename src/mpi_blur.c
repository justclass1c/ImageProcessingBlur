#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../headers/mpi_blur.h"

// Row-strip decomposition: rank 0 sends each worker its rows plus `radius`
// halo rows on each side, every rank blurs its strip, rank 0 gathers.

#define COMMAND_STOP 0
#define COMMAND_BLUR 1

// header: command, width, height, radius, sample_count
#define HEADER_LENGTH 5
#define TAG_STRIP 1

static int world_rank = 0;
static int world_size = 1;

typedef struct {
    int first_row;        // rows this rank blurs
    int row_count;
    int strip_first_row;  // rows it needs as input (own + halo)
    int strip_row_count;
} Part;

// computed identically on every rank, so the layout never has to be sent
static Part part_for_rank(int rank, int size, int height, int radius) {
    Part part;
    int base = height / size;
    int extra = height % size;  // first `extra` ranks take one more row

    part.row_count = base + (rank < extra ? 1 : 0);
    part.first_row = rank * base + (rank < extra ? rank : extra);

    // more ranks than rows
    if (part.row_count == 0) {
        part.strip_first_row = part.first_row;
        part.strip_row_count = 0;
        return part;
    }

    int strip_first = part.first_row - radius;
    int strip_end = part.first_row + part.row_count + radius;

    if (strip_first < 0) strip_first = 0;
    if (strip_end > height) strip_end = height;

    part.strip_first_row = strip_first;
    part.strip_row_count = strip_end - strip_first;
    return part;
}

// Same loops as sequential_blur.c, limited to rows [first_row, first_row + row_count).
// `strip` starts at image row strip_first_row, `output` at first_row.
static void blur_rows(const unsigned char *strip, int strip_first_row,
                      unsigned char *output, int first_row, int row_count,
                      int width, int height, int radius, int sample_count) {
    // strip coordinates (row 0 = first row of the strip), so the inner loop
    // indexes exactly like the sequential version, with no per-sample shift
    int start = first_row - strip_first_row;
    int end = start + row_count;
    int top = -strip_first_row;                  // image row 0
    int bottom = height - 1 - strip_first_row;   // last image row

    for (int x = 0; x < width; x++) {
        for (int y = start; y < end; y++) {
            int sum_r = 0, sum_g = 0, sum_b = 0;

            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    int offset_x = x + dx;
                    int offset_y = y + dy;

                    // clamp to the full image, not the strip
                    if (offset_x < 0) offset_x = 0;
                    if (offset_x >= width) offset_x = width - 1;
                    if (offset_y < top) offset_y = top;
                    if (offset_y > bottom) offset_y = bottom;

                    int offset_pixel = offset_y * width + offset_x;
                    int offset_colour = offset_pixel * 3;

                    sum_r += strip[offset_colour];
                    sum_g += strip[offset_colour + 1];
                    sum_b += strip[offset_colour + 2];
                }
            }

            int output_pixel = ((y - start) * width + x) * 3;
            output[output_pixel] = sum_r / sample_count;
            output[output_pixel + 1] = sum_g / sample_count;
            output[output_pixel + 2] = sum_b / sample_count;
        }
    }
}

// one image row as a datatype, so counts and offsets are in rows
static MPI_Datatype create_row_type(int width) {
    MPI_Datatype row_type;

    MPI_Type_contiguous(width * 3, MPI_UNSIGNED_CHAR, &row_type);
    MPI_Type_commit(&row_type);
    return row_type;
}

int mpi_start(int *argc, char ***argv) {
    MPI_Init(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    return world_rank;
}

int mpi_process_count(void) {
    return world_size;
}

static void worker_blur(int width, int height, int radius, int sample_count) {
    Part part = part_for_rank(world_rank, world_size, height, radius);
    MPI_Datatype row_type = create_row_type(width);

    size_t row_bytes = (size_t)width * 3;
    size_t strip_bytes = (size_t)part.strip_row_count * row_bytes;
    size_t output_bytes = (size_t)part.row_count * row_bytes;

    // malloc(0) may return NULL
    unsigned char *strip = malloc(strip_bytes > 0 ? strip_bytes : 1);
    unsigned char *output = malloc(output_bytes > 0 ? output_bytes : 1);

    if (strip == NULL || output == NULL) {
        // rank 0 is blocked waiting for these rows, so abort the whole job
        fprintf(stderr, "Rank %d: out of memory.\n", world_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Recv(strip, part.strip_row_count, row_type, 0, TAG_STRIP,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    blur_rows(strip, part.strip_first_row, output, part.first_row, part.row_count,
              width, height, radius, sample_count);

    // receive arguments are ignored on non-root ranks
    MPI_Gatherv(output, part.row_count, row_type,
                NULL, NULL, NULL, row_type, 0, MPI_COMM_WORLD);

    free(output);
    free(strip);
    MPI_Type_free(&row_type);
}

void mpi_worker_loop(void) {
    for (;;) {
        int header[HEADER_LENGTH];

        MPI_Bcast(header, HEADER_LENGTH, MPI_INT, 0, MPI_COMM_WORLD);

        if (header[0] == COMMAND_STOP) {
            break;
        }

        worker_blur(header[1], header[2], header[3], header[4]);
    }
}

void mpi_shutdown(void) {
    // release the workers from mpi_worker_loop
    if (world_rank == 0) {
        int header[HEADER_LENGTH] = {COMMAND_STOP, 0, 0, 0, 0};
        MPI_Bcast(header, HEADER_LENGTH, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}

int mpi_blur(const Image *src, Image *dst, const BlurConfig *blur) {
    int width = src->width;
    int height = src->height;
    int radius = blur->diameter / 2;
    int sample_count = blur->sample_count;
    size_t row_bytes = (size_t)width * 3;

    // allocate before the broadcast, so a failure cannot strand the workers mid-job
    int *row_counts = malloc((size_t)world_size * sizeof(int));
    int *row_offsets = malloc((size_t)world_size * sizeof(int));

    if (row_counts == NULL || row_offsets == NULL) {
        fprintf(stderr, "Error: out of memory in mpi_blur.\n");
        free(row_offsets);
        free(row_counts);
        return 1;
    }

    int header[HEADER_LENGTH] = {COMMAND_BLUR, width, height, radius, sample_count};
    MPI_Bcast(header, HEADER_LENGTH, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Datatype row_type = create_row_type(width);

    // rows are contiguous, so each strip is sent straight from the image buffer
    for (int rank = 1; rank < world_size; rank++) {
        Part part = part_for_rank(rank, world_size, height, radius);

        MPI_Send(src->pixels + (size_t)part.strip_first_row * row_bytes,
                 part.strip_row_count, row_type, rank, TAG_STRIP, MPI_COMM_WORLD);

        row_counts[rank] = part.row_count;
        row_offsets[rank] = part.first_row;
    }

    // rank 0 blurs the first strip itself, directly into dst
    Part mine = part_for_rank(0, world_size, height, radius);
    row_counts[0] = mine.row_count;
    row_offsets[0] = mine.first_row;

    blur_rows(src->pixels, 0,
              dst->pixels + (size_t)mine.first_row * row_bytes,
              mine.first_row, mine.row_count,
              width, height, radius, sample_count);

    // MPI_IN_PLACE: rank 0's rows are already in dst
    MPI_Gatherv(MPI_IN_PLACE, 0, row_type,
                dst->pixels, row_counts, row_offsets, row_type,
                0, MPI_COMM_WORLD);

    MPI_Type_free(&row_type);
    free(row_offsets);
    free(row_counts);
    return 0;
}
