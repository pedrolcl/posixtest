#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STRINGIFY2(x) #x
#define STRINGIFY(macro) STRINGIFY2(macro)

static inline int safe_fileno(FILE *f)
{
    fflush(f);
    return fileno(f);
}

static inline void fresync(FILE *f)
{
    off_t off = lseek(safe_fileno(f), 0, SEEK_CUR);
    fseek(f, off, SEEK_SET);
}

int createtestfile(size_t test_size, char *fname)
{
    int ret = EXIT_SUCCESS;
    static char template[] = STRINGIFY(BUILD_DIR) "/testfileXXXXXX";
    strcpy(fname, template);
    int fd = mkstemp(fname);
    if (fd == -1) {
        fprintf(stderr, "Error in mkstemp()");
        return EXIT_FAILURE;
    }
    char *buf = malloc(test_size);
    if (buf == NULL) {
        fprintf(stderr, "Error in malloc()");
        close(fd);
        return EXIT_FAILURE;
    }
    memset(buf, 0, test_size);
    ssize_t count = write(fd, buf, test_size);
    if (count != test_size) {
        fprintf(stderr, "Error in write()");
        ret = EXIT_FAILURE;
    }
    close(fd);
    free(buf);
    return ret;
}

int testfile(const char *filePath)
{
    int ret = EXIT_SUCCESS;
    long fileSize = 0, filePos = 0;
    struct stat st;

    ret = stat(filePath, &st);
    if (ret != 0) {
        fprintf(stderr, "Error stat(%s)\n", filePath);
        return EXIT_FAILURE;
    }

    fileSize = st.st_size;
    filePos = fileSize / 4 * 3;
    fprintf(stdout, "file size: %ld\nfile pos: %ld\n", fileSize, filePos);

    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error fopen(%s)\n", filePath);
        ret = EXIT_FAILURE;
    } else {
        int fd = safe_fileno(file);
        if (fd == -1) {
            fprintf(stderr, "Error fileno(file)\n");
            fclose(file);
            return EXIT_FAILURE;
        }

        off_t pos = lseek(fd, filePos, SEEK_SET);
        if (pos != filePos) {
            fprintf(stderr, "Error lseek(fd,%ld)->%d\n", filePos, pos);
            fclose(file);
            return EXIT_FAILURE;
        }
        fresync(file);

        pos = ftell(file);
        if (pos != filePos) {
            fprintf(stderr, "Error ftell(file)->%ld and filePos=%ld\n", pos, filePos);
            fclose(file);
            return EXIT_FAILURE;
        }

        pos = lseek(fd, 0, SEEK_CUR);
        if (pos != filePos) {
            fprintf(stderr, "Error lseek(fd)->%ld and filePos=%ld\n", pos, filePos);
            fclose(file);
            return EXIT_FAILURE;
        }

        int dupfd = dup(fd);
        if (dupfd == -1) {
            fprintf(stderr, "Error dup(fd)\n");
            fclose(file);
            return EXIT_FAILURE;
        }

        pos = lseek(dupfd, 0, SEEK_CUR);
        if (pos != filePos) {
            fprintf(stderr, "Error lseek(dupfd)->%ld and filePos=%ld\n", pos, filePos);
            fclose(file);
            return EXIT_FAILURE;
        }

        FILE *dupfile = fdopen(dupfd, "rb");
        if (dupfile == NULL) {
            fprintf(stderr, "Error fdopen(dupfile)\n");
            fclose(file);
            return EXIT_FAILURE;
        }

        pos = ftell(dupfile);
        if (pos != filePos) {
            fprintf(stderr, "Error ftell(dupfile)->%ld and filePos=%ld\n", pos, filePos);
            fclose(file);
            return EXIT_FAILURE;
        }

        pos = lseek(dupfd, 0, SEEK_CUR);
        if (filePos != pos) {
            fprintf(stderr, "Error lseek(dupfd)->%ld and filePos=%ld\n", pos, filePos);
            fclose(file);
            return EXIT_FAILURE;
        }

        ret = fclose(file);
    }
    return ret;
}

int main(int argc, char **argv)
{
    char fname[PATH_MAX];

    if (argc < 2) {
        fprintf(stderr, "Error: test file size argument required!\n");
        return EXIT_FAILURE;
    }
    size_t test_size = atol(argv[1]);

    int ret = createtestfile(test_size, fname);
    if (ret == EXIT_SUCCESS) {
        printf("\nTest %ld : %s\n", test_size, fname);
        ret = testfile(fname);
        if (ret != EXIT_SUCCESS) {
            fprintf(stderr, "Test failed!\n");
        }
    } else {
        fprintf(stderr, "Test file %s creation failed!\n", fname);
    }

    unlink(fname);
    return ret;
}
