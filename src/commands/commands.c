//
// Created by Jerry on 11/23/2024.
//

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../utils/file/file.h"
#include "../utils/compression/compression.h"

int int_git() {
    if (mkdir(GIT_ROOT_DIR, DIRECTORY_PERMISSION) == -1
        || mkdir(GIT_REFS_DIR, DIRECTORY_PERMISSION) == -1
        || mkdir(GIT_OBJECTS_DIR, DIRECTORY_PERMISSION) == -1) {
        fprintf(stderr, "Failed to create directories: %s\n", strerror(errno));
        return 1;
        }

    FILE *headFile = fopen(".git/HEAD", "w");
    if (headFile == NULL) {
        fprintf(stderr, "Failed to create .git/HEAD file: %s\n", strerror(errno));
        return 1;
    }
    fprintf(headFile, "ref: refs/heads/main\n");
    fclose(headFile);

    printf("Initialized git directory\n");

    return 0;
}

int cat_file(const char *path) {
    long uncompressed_size;
    long compressed_size;

    char *compressed_file = read_git_blob_file(path, &compressed_size);
    if (compressed_file == NULL) {
        printf("Error reading file %s\n", path);
        free(compressed_file);

        return 1;
    }
    unsigned char *decompressed_file = decompress_data(compressed_file, compressed_size, &uncompressed_size);
    if (decompressed_file != NULL) {
        unsigned char *header_end = memchr(decompressed_file, '\0', uncompressed_size);
        if (header_end == NULL) {
            printf("Error reading git file header%s\n", path);
            free(compressed_file);
            free(decompressed_file);

            return 1;
        }

        unsigned char *body_content = header_end + 1;
        const unsigned long body_size = uncompressed_size - (body_content - decompressed_file);
        printf("%.*s", (int)body_size, body_content);
    }

    free(compressed_file);
    free(decompressed_file);

    return 0;
}

int hash_object(const char* path) {
    long file_size;
    long compressed_size;

    char *file = read_file(path, &file_size);
    if(file == NULL) {
        printf("Error reading file %s\n", path);
        free(file);

        return 1;
    }

    unsigned char *compressed_file_data = compress_data(file, file_size, &compressed_size);
    if(compressed_size == 0) {
        free(file);

        return 1;
    }

    if(write_file(compressed_file_data, file)) {
        free(file);

        return 1;
    }

}