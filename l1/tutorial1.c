#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 101

void scan_cwd(void) {
    DIR* dir = opendir(".");
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int dev_blk = 0;
    int dev_chr = 0;
    int ino_dir = 0;
    int ino_fifo = 0;
    int ino_reg = 0;
    int ino_link = 0;
    int ino_sock = 0;

    while (1) {
        errno = 0;
        struct dirent* entry = readdir(dir);
        
        if (!entry) {
            if (errno) {
                perror("readdir");
                goto fail;
            }

            break;
        }

        struct stat stat;
        if(lstat(entry->d_name, &stat) != 0) {
            perror("lstat");
            goto fail;
        }

        mode_t mode = stat.st_mode;

        if (S_ISBLK(mode)) dev_blk++;
        if (S_ISCHR(mode)) dev_chr++;
        if (S_ISDIR(mode)) ino_dir++;
        if (S_ISFIFO(mode)) ino_fifo++;
        if (S_ISREG(mode)) ino_reg++;
        if (S_ISLNK(mode)) ino_link++;
        if (S_ISSOCK(mode)) ino_sock++;
    }

    printf("Regular files: %3d.       Directories: %3d.\n", ino_reg, ino_dir);
    printf("Block devices: %3d. Character devices: %3d.\n", dev_blk, dev_chr);
    printf("FIFOs: %3d.   Symlinks: %3d.  Sockets: %3d.\n", ino_fifo, ino_link, ino_sock);

    if (closedir(dir) != 0) {
        perror("closedir");
    }

    return;

fail:
    if (closedir(dir) != 0) {
        perror("closedir");
    }

    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("Statistics for directory .:\n");
        scan_cwd();
        return EXIT_SUCCESS;
    }

    char cwd[MAX_PATH];

    if(!getcwd(cwd, MAX_PATH)) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        const char* path = argv[i];

        if (i > 1)
            printf("\n");

        if(chdir(path) != 0) {
            fprintf(stderr, "Failed to chdir(%s): %s\n", path, strerror(errno));
            continue;
        }

        printf("Statistics for directory %s:\n", path);

        scan_cwd();        

        if(chdir(cwd) != 0) {
            perror("chdir");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}