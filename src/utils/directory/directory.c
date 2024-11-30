//
// Created by Jerry on 11/30/2024.
//

#include <stdio.h>
#include <sys/stat.h>

int is_directory_present(const char *path) {
    if (path == NULL) {
        printf("Provided directory not found %s\n", path);

        return 1;
    }

    struct stat statbuf;
    if(stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        return 0;
    }

    return 1;
}

