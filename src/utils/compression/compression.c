//
// Created by Jerry on 11/23/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

unsigned char *decompress_data(const unsigned char *compressed_data, const unsigned long compressed_data_size,
                               unsigned long *decompressed_data_size) {
    *decompressed_data_size = compressed_data_size * 4;
    unsigned char *decompressed_data = malloc(*decompressed_data_size);
    if (decompressed_data == NULL) {
        printf("malloc failed\n");

        return NULL;
    }

    const int result = uncompress(decompressed_data, decompressed_data_size, compressed_data, compressed_data_size);
    if (result != Z_OK) {
        printf("uncompress failed\n");
        free(decompressed_data);

        return NULL;
    }

    return decompressed_data;
}

unsigned char *compress_data(const unsigned char *file_data, unsigned long file_data_size,
                             unsigned long *compressed_data_size) {
    if (!file_data || file_data_size == 0 || !compressed_data_size) {
        return NULL;
    }

    uLongf max_compressed_data_size = compressBound(file_data_size);
    unsigned char *compressed_data = malloc(max_compressed_data_size);
    if (!compressed_data) {
        printf("malloc failed\n");
        free(compressed_data);

        return NULL;
    }

    int result = compress(compressed_data, &max_compressed_data_size, file_data, file_data_size);
    if (result != Z_OK) {
        printf("compress failed\n");
        free(compressed_data);

        return NULL;
    }

    *compressed_data_size = max_compressed_data_size;

    return compressed_data;
}
