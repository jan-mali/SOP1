#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

void usage(const char* argv0);
void make_file(const char* path, mode_t mode, size_t size, int percent);

int main(int argc, char** argv) {
    char c, *filename, *perm_str, *size_str;

    int has_filename = 0;
    int has_perm_str = 0;
    int has_size_str = 0;
    int err = 0;

    while ((c = getopt(argc, argv, ":n:p:s:")) != -1) {
        switch (c) {
            case 'n':
                // Name
                filename = optarg;
                if (has_filename) {
                    fprintf(stderr, "Only one -n option may be specified\n");
                    err++;
                }
                has_filename++;
                break;
            case 'p':
                // Permissions
                perm_str = optarg;
                if (has_perm_str) {
                    fprintf(stderr, "Only one -p option may be specified\n");
                    err++;
                }
                has_perm_str++;
                break;
            case 's':
                // Size
                size_str = optarg;
                if (has_size_str) {
                    fprintf(stderr, "Only one -s option may be specified\n");
                    err++;
                }
                has_size_str++;
                break;
            case ':':
                fprintf(stderr, "Option -%c requires an operand\n", optopt);
                err++;
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: -%c\n", optopt);
                err++;
                break;
        }
    }

    if (!has_filename) {
        fprintf(stderr, "Filename wasn't specified\n");
        err++;
    }

    if (!has_perm_str) {
        fprintf(stderr, "Permission number wasn't specified\n");
        err++;
    }

    if (!has_size_str) {
        fprintf(stderr, "File size wasn't specified\n");
        err++;
    }

    if (err > 0) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    mode_t permissions = 0;
    ssize_t size = 0;

    errno = 0;
    int raw_permissions = strtol(perm_str, NULL, 8);
    if (errno) {
        fprintf(stderr, "Failed to parse permission string: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    permissions = raw_permissions & 0777;

    errno = 0;
    size = strtoll(size_str, NULL, 10);
    if (errno) {
        fprintf(stderr, "Failed to parse size string: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    make_file(filename, permissions, size, 10);

    return EXIT_SUCCESS;
}

void usage(const char* argv0) {
    fprintf(stderr, "Usage: %s -n <name> -p <permissions> -s <size>\n", argv0);
}

void make_file(const char* path, mode_t mode, size_t size, int percent) {
    // printf("Path: %s\n", path);
    // printf("Mode: %04o\n", mode);
    // printf("Size: %zd\n", size);

    struct stat path_stat;

    errno = 0;

    if (lstat(path, &path_stat) == 0) {
        // Remove the file
        if (unlink(path) != 0) {
            fprintf(stderr, "Failed to unlink %s: %s\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (errno != ENOENT) {
        // Stat failed but not because the fail doesn't exist
        fprintf(stderr, "Failed to stat %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Here the file is guaranteed not to exist (I guess it is a TOCTOU race condition if another process messes with the file but let's ignore that for now)

    umask(~mode & 0777);

    FILE* file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "Failed to fopen %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fseek(file, size - 1, SEEK_SET) != 0) {
        perror("fseek");
        goto fail;
    }

    if (fputc(0, file) != 0) {
        perror("fputc");
        goto fail;
    }

    for (int i = 0; i < (size * percent) / 100; i++) {
        if (fseek(file, rand() % size, SEEK_SET) != 0) {
            perror("fseek");
            goto fail;
        }

        if (fprintf(file, "%c", 'A' + rand() % ('Z' - 'A' + 1)) != 1) {
            fprintf(stderr, "Failed to write to the file\n");
            goto fail;
        }
    }
    
    if (fclose(file) != 0) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    if (fchmodat(AT_FDCWD, path, mode & 0777, AT_SYMLINK_NOFOLLOW) != 0) {
        fprintf(stderr, "Failed to set the mode %04d on file %s: %s\n", mode & 0777, path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return;

fail:
    if (fclose(file) != 0) {
        perror("fclose");
    }

    exit(EXIT_FAILURE);
}