#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(void) {
    DIR* dir = opendir(".");
    if (!dir) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    int ret = EXIT_SUCCESS;

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
                ret = EXIT_FAILURE;
                goto cleanup;
            }

            break;
        }

        struct stat stat;
        if(lstat(entry->d_name, &stat) != 0) {
            perror("lstat");
            ret = EXIT_FAILURE;
            goto cleanup;
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

cleanup:

    if (closedir(dir) != 0) {
        perror("closedir");
        ret = EXIT_FAILURE;
    }

    return ret;
}