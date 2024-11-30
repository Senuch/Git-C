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

    // Allocate enough memory for "xx/xxxxxxxxxx..." + null terminator
    char *file_path = malloc(sha_length + 3); // +2 for "xx/" and +1 for '\0'
    if (file_path == NULL) {
        perror("malloc failed");
        return NULL;
    }

    // Copy the first 2 characters to form the directory name
    strncpy(file_path, file_name, 2); // First 2 characters
    file_path[2] = '/';              // Add the separator '/'

    // Copy the remaining part of the hash as the file name
    strcpy(file_path + 3, file_name + 2); // Rest of the hash string

    return file_path; // Caller must free this memory
}

int split_file_path(const char *file_path, char **directory, char **file_name) {
    const char *last_slash = strrchr(file_path, '/');
    if (last_slash == NULL) {
        printf("File path is invalid\n");
        return 1;
    }

    // Calculate lengths
    size_t directory_len = last_slash - file_path;
    size_t file_name_len = strlen(file_path) - directory_len - 1;

    // Allocate memory for directory
    *directory = (char *)malloc(directory_len + 1);
    if (*directory == NULL) {
        perror("Failed to allocate memory for directory");
        return 1;
    }

    // Allocate memory for file name
    *file_name = (char *)malloc(file_name_len + 1);
    if (*file_name == NULL) {
        perror("Failed to allocate memory for file name");
        free(directory); // Free previously allocated memory
        return 1;
    }

    // Copy directory
    strncpy(*directory, file_path, directory_len);
    (*directory)[directory_len] = '\0';

    // Copy file name
    strncpy(*file_name, last_slash + 1, file_name_len);
    (*file_name)[file_name_len] = '\0';

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

int write_file(const char *file_absolute_path, const char *data, size_t byte_count, char const *wm) {
    FILE *target_file = fopen(file_absolute_path, wm);
    if (target_file == NULL) {
        printf("Error opening file %s\n", file_absolute_path);

        return 1;
    }

    fwrite(data, sizeof(char), byte_count, target_file);
    fclose(target_file);

    return 0;
}
