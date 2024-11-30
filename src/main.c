#include <stdio.h>
#include <string.h>
#include "commands/commands.h"

int main(const int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 2) {
        fprintf(stderr, "Usage: ./your_program.sh <command> [<args>]\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "init") == 0) {
        return int_git();
    }

    if (strcmp(command, "cat-file") == 0 && (strcmp(argv[2], "-p") == 0 || strlen(argv[3]) > 0)) {
        return cat_file(argv[3]);
    }

    if(strcmp(command, "hash-object") == 0 && (strcmp(argv[2], "-w") == 0 || strlen(argv[3]) > 0)) {
        int res = hash_object(argv[3]);
        return res;
    }

    fprintf(stderr, "Unknown command %s\n", command);
    return 1;
}
