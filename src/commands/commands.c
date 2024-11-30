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
#include <zconf.h>
#include <zlib.h>
#include <openssl/sha.h>

#include "../constants.h"
#include "../utils/file/file.h"
#include "../utils/compression/compression.h"
#include "../utils/string/string.h"
#include "../utils/directory/directory.h"

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
        printf("%.*s", (int) body_size, body_content);
    }

    free(compressed_file);
    free(decompressed_file);

    return 0;
}

int hash_object(const char *path) {
    long file_size;

    char *file_content = read_file(path, &file_size);
    if (file_content == NULL) {
        printf("Error reading file %s\n", path);
        free(file_content);

        return 1;
    }

    char file_data_to_compress[FILE_BUFFER_SIZE];
    sprintf(file_data_to_compress, "blob %d", file_size);
    size_t header_length = strlen(file_data_to_compress);
    file_data_to_compress[header_length] = '\0';
    memcpy(file_data_to_compress + header_length + 1, file_content, file_size + 1);

    unsigned char hex_hash_output[SHA_DIGEST_LENGTH];
    SHA1(file_data_to_compress, header_length + 1 + file_size, hex_hash_output);
    unsigned char *str_hash_output = hex_to_string(hex_hash_output, SHA_DIGEST_LENGTH);

    printf("%s\n", str_hash_output);

    char *dir_name = NULL;
    char *file_name = NULL;
    char absolute_dir_path[PATH_MAX];
    char absolute_file_path[PATH_MAX];
    char *file_path = get_file_path(str_hash_output);
    if (split_file_path(file_path, &dir_name, &file_name) == 1) {
        printf("Error splitting file %s\n", dir_name);
        free(dir_name);
        free(file_name);

        return 1;
    }

    strcpy(absolute_dir_path, GIT_OBJECTS_DIR);
    strcat(absolute_dir_path, "/");
    strcat(absolute_dir_path, dir_name);

    if (is_directory_present(absolute_dir_path) == 1 && mkdir(absolute_dir_path, DIRECTORY_PERMISSION) == -1) {
        printf("Error mkdir %s\n", dir_name);
        free(dir_name);
        free(file_name);

        return 1;
    }

    strcpy(absolute_file_path, GIT_OBJECTS_DIR);
    strcat(absolute_file_path, "/");
    strcat(absolute_file_path, dir_name);
    strcat(absolute_file_path, "/");
    strcat(absolute_file_path, file_name);

    // Allocate memory for compressed data
    unsigned char compressed_data[FILE_BUFFER_SIZE];
    uLongf compressed_size = sizeof(compressed_data);

    // Compress the data
    int result = compress(compressed_data, &compressed_size,
                          file_data_to_compress, header_length + 1 + file_size);

    if (result != Z_OK) {
        fprintf(stderr, "Compression failed: %d\n", result);
        return 1;
    }

    if(write_file(absolute_file_path, compressed_data, compressed_size, "ab") == 1) {
        printf("Error writing %s\n", file_name);
        free(file_content);
        free(file_name);

        return 1;
    }

    free(file_content);
    free(file_name);
    free(dir_name);
    free(file_path);

    return 0;
}
