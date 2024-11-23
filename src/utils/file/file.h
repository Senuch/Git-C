//
// Created by Jerry on 11/23/2024.
//

#ifndef FILE_H
#define FILE_H

char *get_file_path(const char *file_name);
int write_file(const char *sha1_string, const char *data);
char *read_file(const char *file_absolute_path, long *file_size);
char *read_git_blob_file(const char *sha1_string, long *compressed_size);

#endif //FILE_H
