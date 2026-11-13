#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image.h"
#include "../stb_image_write.h"
#include "../headers/image.h"

int loadImage(Image *image, const char *path) {
    int file_channels = 0;

    image->pixels = stbi_load(path, &image->width, &image->height, &file_channels, 3);
    image->channels = 3;

    return image->pixels == NULL;
}

int image_save_png(const char *path, const Image *image) {
    // create the parent directory if it is missing, e.g. "output" for "output/...png"
    // mkdir returns EEXIST when it is already there, which counts as success
    const char *slash = strrchr(path, '/');
    if (slash != NULL && slash != path) {
        size_t dir_length = (size_t)(slash - path);
        char directory[512];

        if (dir_length >= sizeof(directory)) {
            fprintf(stderr, "Error: directory path too long for '%s'\n", path);
            return 1;
        }

        memcpy(directory, path, dir_length);
        directory[dir_length] = '\0';

#ifdef _WIN32
        if (_mkdir(directory) != 0 && errno != EEXIST) {
#else
        if (mkdir(directory, 0755) != 0 && errno != EEXIST) {
#endif
            fprintf(stderr, "Error: cannot create directory '%s': %s\n", directory, strerror(errno));
            return 1;
        }
    }

    // stbi_write_png returns 0 on failure (e.g. missing directory, no disk space)
    if (stbi_write_png(path, image->width, image->height, image->channels,
                       image->pixels, image->width * image->channels) == 0) {
        fprintf(stderr, "Error: failed to write '%s'\n", path);
        return 1;
    }

    return 0;
}

void image_free(Image *image) {
    stbi_image_free(image->pixels);
    image->pixels = NULL;
}

int getImageNames(char imageNames[][MAX_NAME_LENGTH]) {
    int imageCount = 0;

#ifdef _WIN32
    WIN32_FIND_DATAA entry;
    HANDLE dir = FindFirstFileA("images\\*", &entry);

    if (dir == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "opendir: cannot open the images directory\n");
        return -1;
    }

    do {
        if (imageCount >= MAX_IMAGES) break;

        // skip directories and hidden files, same as the dirent version
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (entry.cFileName[0] == '.') continue;

        int character;
        for (character = 0; character < MAX_NAME_LENGTH - 1 && entry.cFileName[character] != '\0'; character++) {
            imageNames[imageCount][character] = entry.cFileName[character];
        }
        imageNames[imageCount][character] = '\0';

        imageCount++;
    } while (FindNextFileA(dir, &entry));

    FindClose(dir);
#else
    DIR *dir = opendir("images");

    if (!dir) {
        perror("opendir"); // print error with not found
        return -1;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (imageCount >= MAX_IMAGES) break;

        // skip . and .. directories
        if (entry->d_name[0] == '.') {
            continue;
        }

        int character;
        for (character = 0; character < MAX_NAME_LENGTH - 1; character++) {
            imageNames[imageCount][character] = entry->d_name[character];
            if (entry->d_name[character] == '\0') break;
        }
        imageNames[imageCount][character] = '\0';

        imageCount++;
    }

    closedir(dir);
#endif

    for (int i = 0; i < imageCount; i++) {
        printf("%d. %s\n", i + 1, imageNames[i]);
    }

    return imageCount;
}

const char *image_error(void) {
    return stbi_failure_reason();
}
