//
// Created by Jerry on 11/23/2024.
//

#include <stdio.h>
#include <stdlib.h>

void slice_str(const char *str, char *buffer, const size_t start, const size_t end) {
    size_t index = 0;
    for (size_t i = start; i < end; i++) {
        buffer[index++] = str[i];
    }

    buffer[index] = '\0';
}

void size_t_to_string(size_t value, char *str, size_t buffer_size) {
    snprintf(str, buffer_size, "%zu", value);
}

char *hex_to_string(const unsigned char *buffer, size_t buffer_size) {
    char *str = malloc(buffer_size * 2  + 1);
    if(str == NULL) {
        printf("Failed to allocate memory!\n");
        return NULL;
    }

    for (size_t i = 0; i < buffer_size; i++) {
        sprintf(str + (i * 2), "%02x", buffer[i]);
    }

    str[buffer_size * 2] = '\0';
    return str;
}