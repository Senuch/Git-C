//
// Created by Jerry on 11/23/2024.
//

#ifndef COMPRESSION_H
#define COMPRESSION_H

unsigned char *decompress_data(const unsigned char *compressed_data, unsigned long compressed_data_size,
                               unsigned long *decompressed_data_size);
unsigned char *compress_data(const unsigned char *file_data, unsigned long file_data_size,
                               unsigned long *compressed_data_size);

#endif //COMPRESSION_H
