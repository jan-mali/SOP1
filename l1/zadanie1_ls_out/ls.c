#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

void list_cwd(FILE* output_stream, const char* path);

int main(int argc, char** argv) {
    int c;
    int printed_count = 0;

    while ((c = getopt(argc, argv, ":p:")) != -1) {
        switch (c) {
        case 'p':
            if (printed_count > 0) {
                fprintf(stdout, "\n");
            }

            list_cwd(stdout, optarg);
            printed_count++;

            break;
        }
    }

    if (!printed_count){
        list_cwd(stdout, ".");
    }

    return EXIT_SUCCESS;
}

void list_cwd(FILE* output_stream, const char* path) {
    struct stat file_stat;
    struct dirent* dir_entry;
    DIR* dir_stream;

    if ((dir_stream = opendir(path)) == NULL) {
        perror("Failed to open the directory");
        exit(EXIT_FAILURE);
    }

    fprintf(output_stream, "PATH: %s\n", path);
    fprintf(output_stream, "FILES:\n");

    do {
        errno = 0;
        dir_entry = readdir(dir_stream);
        
        if (!dir_entry) break;

        int at_fd = dirfd(dir_stream);
        if (at_fd == -1) {
            perror("dirfd");
            goto fail;
        }

        if (fstatat(at_fd, dir_entry->d_name, &file_stat, AT_SYMLINK_NOFOLLOW) != 0) {
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