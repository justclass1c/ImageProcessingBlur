#ifndef MPI_BLUR_H
#define MPI_BLUR_H

#include "image.h"
#include "blur.h"

int mpi_start(int *argc, char ***argv);

int mpi_process_count(void);

void mpi_worker_loop(void);

void mpi_shutdown(void);

int mpi_blur(const Image *src, Image *dst, const BlurConfig *blur);

#endif
