#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif
#include <errno.h>

void slice_str(const char *str, char *buffer, size_t start, size_t end) {
    size_t index = 0;
    for (size_t i = start; i < end; i++) {
        buffer[index++] = str[i];
    }

    buffer[index] = '\0';
}

char *get_file_path(const char *file_name) {
    const size_t sha_length = strlen(file_name);
    char *file_path = malloc(sha_length * sizeof(char));
    char buffer[sha_length + 1];
    slice_str(file_name, buffer, 0, 1);
    strcpy(file_path, buffer);
    strcat(file_path, "/");
    slice_str(file_name, buffer, 2, sha_length - 1);
    strcat(file_path, buffer);

    return file_path;
}

unsigned char *decompress_data(const unsigned char *compressed_data, uLong compressed_data_size,
                               unsigned long *decompressed_data_size) {
    *decompressed_data_size = compressed_data_size * 4;
    unsigned char *decompressed_data = malloc(*decompressed_data_size);
    if (decompressed_data == NULL) {
        printf("malloc failed\n");

        return nullptr;
    }

    int result = uncompress(decompressed_data, decompressed_data_size, compressed_data, compressed_data_size);
    if (result != Z_OK) {
        printf("uncompress failed\n");
        free(decompressed_data);

        return nullptr;
    }

    return decompressed_data;
}

char *read_file(const char *sha1_string, long *compressed_size) {
    char *file_relative_path = get_file_path(sha1_string);
    const char *git_default_objects_path = ".git/objects/";
    const char file_absolute_path[strlen(file_relative_path) + strlen(git_default_objects_path)];
    strcpy(file_absolute_path, git_default_objects_path);
    strcat(file_absolute_path, file_relative_path);

    FILE *target_file = fopen(file_absolute_path, "rb");
    if (target_file == NULL) {
        printf("Error opening file %s\n", file_absolute_path);
        return nullptr;
    }

    if (fseek(target_file, 0, SEEK_END) != 0) {
        printf("Error seeking end of file %s\n", file_absolute_path);
        fclose(target_file);
        return nullptr;
    }

    const long file_size = ftell(target_file);
    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        printf("Error allocating memory for file content\n");
        fclose(target_file);
        free(file_content);

        return nullptr;
    }

    if (fseek(target_file, 0, SEEK_SET) != 0) {
        printf("Error seeking end of file %s\n", file_absolute_path);
        fclose(target_file);
        free(file_content);

        return nullptr;
    }

    size_t file_bytes_read = fread(file_content, sizeof(char), file_size, target_file);
    if (file_bytes_read != file_size) {
        printf("Error reading file %s\n", file_absolute_path);
        fclose(target_file);
        free(file_content);

        return nullptr;
    }

    *compressed_size = (long) file_bytes_read;
    file_content[file_size] = '\0';
    fclose(target_file);

    free(file_relative_path);

    return file_content;
}

void cat_file(const char *path) {
    long uncompressed_size;
    long compressed_size;

    char *compressed_file = read_file(path, &compressed_size);
    if (compressed_file == NULL) {
        printf("Error reading file %s\n", path);
        free(compressed_file);

        return;
    }
    unsigned char *decompressed_file = decompress_data(compressed_file, compressed_size, &uncompressed_size);
    if (decompressed_file != NULL) {
        unsigned char *header_end = memchr(decompressed_file, '\0', uncompressed_size);
        if (header_end == NULL) {
            printf("Error reading git file header%s\n", path);
            free(compressed_file);
            free(decompressed_file);

            return;
        }

        unsigned char *body_content = header_end + 1;
        const uLong body_size = uncompressed_size - (body_content - decompressed_file);
        printf("%.*s", (int)body_size, body_content);
    }

    free(compressed_file);
    free(decompressed_file);
}

int main(const int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);

    if (argc < 2) {
        fprintf(stderr, "Usage: ./your_program.sh <command> [<args>]\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "init") == 0) {
        if (mkdir(".git", 0755) == -1 ||
            mkdir(".git/objects", 0755) == -1 ||
            mkdir(".git/refs", 0755) == -1) {
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
    } else if (strcmp(command, "cat-file") == 0 && (strcmp(argv[2], "-p") == 0 || strlen(argv[3]) > 0)) {
        cat_file(argv[3]);
    } else {
        fprintf(stderr, "Unknown command %s\n", command);
        return 1;
    }

    return 0;
}
