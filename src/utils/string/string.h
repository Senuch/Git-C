//
// Created by Jerry on 11/23/2024.
//

#ifndef STRING_H
#define STRING_H

void size_t_to_string(size_t value, char *str, size_t buffer_size);
void slice_str(const char *str, char *buffer, size_t start, size_t end);
char *hex_to_string(const unsigned char *buffer, size_t buffer_size);

#endif //STRING_H
