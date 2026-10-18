#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

void list_cwd(FILE* output_stream, const char* path);

int main(int argc, char** argv) {
    list_cwd(stdout, ".");
}

void list_cwd(FILE* output_stream, const char* path) {
    struct stat file_stat;
    struct dirent* dir_entry;
    DIR* dir_stream;

    // TODO: Use the path parameter

    if ((dir_stream = opendir(".")) == NULL) {
        perror("Failed to open the directory");
        exit(EXIT_FAILURE);
    }

    fprintf(output_stream, "PATH: %s\n", path);
    fprintf(output_stream, "FILES:\n");

    do {
        errno = 0;
        dir_entry = readdir(dir_stream);
        
        if (!dir_entry) break;

        if (lstat(dir_entry->d_name, &file_stat) != 0) {
            fprintf(stderr, "Failed to stat %s: %s\n", dir_entry->d_name, strerror(errno));
            goto fail;
        }

        mode_t mode = file_stat.st_mode;

        fprintf(output_stream, "%6s", dir_entry->d_name);
        if (S_ISREG(mode)) {
            fprintf(output_stream, " %4ld\n", file_stat.st_size);
        } else {
            fprintf(output_stream, "\n");
        }
    } while(dir_entry != NULL);

    if (errno) {
        perror("Failed to read from the directory");
        exit(EXIT_FAILURE);
    }

fail:

    if (closedir(dir_stream) != 0) {
        perror("Failed to close the directory");
        exit(EXIT_FAILURE);
    }
}