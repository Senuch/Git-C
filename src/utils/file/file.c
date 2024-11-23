//
// Created by Jerry on 11/23/2024.
//

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../constants.h"
#include "../string/string.h"

char *get_file_path(const char *file_name) {
    const size_t sha_length = strlen(file_name);
    char *file_path = malloc(sha_length * sizeof(char));
    char buffer[sha_length + 1];
    slice_str(file_name, buffer, 0, 2);
    strcpy(file_path, buffer);
    strcat(file_path, "/");
    slice_str(file_name, buffer, 2, sha_length);
    strcat(file_path, buffer);

    return file_path;
}

int split_file_path(const char *file_path, char *directory, char *file_name) {
    const char *last_slash = strrchr(file_path, '/');
    if (last_slash == NULL) {
        printf("File path is invalid\n");
        return 1;
    }

    // Calculate lengths
    size_t directory_len = last_slash - file_path;
    size_t file_name_len = strlen(file_path) - directory_len - 1;

    // Allocate memory for directory
    directory = (char *)malloc(directory_len + 1);
    if (directory == NULL) {
        perror("Failed to allocate memory for directory");
        return 1;
    }

    // Allocate memory for file name
    file_name = (char *)malloc(file_name_len + 1);
    if (file_name == NULL) {
        perror("Failed to allocate memory for file name");
        free(directory); // Free previously allocated memory
        return 1;
    }

    // Copy directory
    strncpy(directory, file_path, directory_len);
    directory[directory_len] = '\0';

    printf("Directory: %s\n", directory);

    // Copy file name
    strncpy(file_name, last_slash + 1, file_name_len);
    file_name[file_name_len] = '\0';

    printf("File name: %s\n", file_name);

    return 0;
}

char *read_file(const char *file_absolute_path, long *file_size) {
    FILE *target_file = fopen(file_absolute_path, "rb");
    if (target_file == NULL) {
        printf("Error opening file %s\n", file_absolute_path);
        return NULL;
    }

    if (fseek(target_file, 0, SEEK_END) != 0) {
        printf("Error seeking end of file %s\n", file_absolute_path);
        fclose(target_file);
        return NULL;
    }

    const long target_file_size = ftell(target_file);
    char *file_content = malloc(target_file_size + 1);
    if (file_content == NULL) {
        printf("Error allocating memory for file content\n");
        fclose(target_file);
        free(file_content);

        return NULL;
    }

    if (fseek(target_file, 0, SEEK_SET) != 0) {
        printf("Error seeking end of file %s\n", file_absolute_path);
        fclose(target_file);
        free(file_content);

        return NULL;
    }

    size_t file_bytes_read = fread(file_content, sizeof(char), target_file_size, target_file);
    if (file_bytes_read != target_file_size) {
        printf("Error reading file %s\n", file_absolute_path);
        fclose(target_file);
        free(file_content);

        return NULL;
    }

    *file_size = target_file_size;
    file_content[target_file_size] = '\0';
    fclose(target_file);

    return file_content;
}

char *read_git_blob_file(const char *sha1_string, long *compressed_size) {
    const char *file_relative_path = get_file_path(sha1_string);
    const char *git_default_objects_path = GIT_OBJECTS_DIR;
    const char file_absolute_path[strlen(file_relative_path) + strlen(git_default_objects_path)];
    strcpy(file_absolute_path, git_default_objects_path);
    strcat(file_absolute_path, file_relative_path);

    long file_size = 0;
    char *file_content = read_file(file_absolute_path, &file_size);
    if (file_content == NULL) {
        printf("Error reading file %s\n", file_absolute_path);
        free(file_content);

        return NULL;
    }

    *compressed_size = file_size;

    return file_content;
}

int write_file(const char *sha1_string, const char *data) {
    char *dir_name = NULL;
    char *file_name = NULL;
    char *file_path = get_file_path(sha1_string);

    if (split_file_path(file_path, dir_name, file_name) == 1) {
        printf("Error splitting file %s\n", dir_name);
        free(dir_name);
        free(file_name);

        return 1;
    }

    if (mkdir(dir_name, DIRECTORY_PERMISSION) == -1) {
        printf("Error mkdir %s\n", dir_name);
        free(dir_name);
        free(file_name);

        return 1;
    }

    const char *git_default_objects_path = GIT_OBJECTS_DIR;
    const char file_absolute_path[strlen(sha1_string) + strlen(git_default_objects_path)];
    strcpy(file_absolute_path, git_default_objects_path);
    strcat(file_absolute_path, sha1_string);

    FILE *target_file = fopen(file_absolute_path, "wb");
    if (target_file == NULL) {
        printf("Error opening file %s\n", file_absolute_path);
        free(file_absolute_path);
        free(dir_name);
        free(file_name);

        return 1;
    }

    char *blob_prefix = "blob ";
    size_t data_len = strlen(data);
    char data_len_str[20];
    size_t_to_string(data_len, data_len_str, sizeof(data_len_str));

    char *buffer = malloc(strlen(blob_prefix) + strlen(data_len_str) + 1);
    if (buffer == NULL) {
        printf("Error allocating memory for buffer\n");
        free(file_absolute_path);
        free(dir_name);
        free(file_name);

        return 1;
    }

    strcpy(buffer, blob_prefix);
    strcat(buffer, data);
    buffer[strlen(buffer) - 1] = '\0';

    fwrite(buffer, sizeof(char), strlen(buffer), target_file);
    fclose(target_file);
    free(buffer);

    target_file = fopen(file_absolute_path, "ab");
    if (target_file == NULL) {
        printf("Error opening file %s\n", file_absolute_path);
        free(file_absolute_path);
        free(dir_name);
        free(file_name);

        return 1;
    }

    size_t data_length = strlen(data);
    size_t file_bytes_written = fwrite(data, sizeof(char), data_length, target_file);
    if (file_bytes_written != data_length) {
        printf("Error writing to file %s\n", file_absolute_path);
        free(file_absolute_path);
        free(dir_name);
        free(file_name);

        return 1;
    }

    fclose(target_file);
    free(dir_name);
    free(file_name);

    return 0;
}
