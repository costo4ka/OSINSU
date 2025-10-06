#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

static void print_ids(const char *stage) {
    uid_t r = getuid();
    uid_t e = geteuid();
    printf("[%s] ruid=%ld euid=%ld\n", stage, (long)r, (long)e);
}

// файл существует и имеет право доступа 0600
static int make_file_private(const char *path) {
    mode_t old = umask(000);
    FILE *f = fopen(path, "a");
    int saved_errno = 0;

    //если файл не открылся
    if (!f) {
        saved_errno = errno;
        umask(old);
        errno = saved_errno;
        return -1;
    }
    // если файл не закрылся
    if (fclose(f) != 0) {
        saved_errno = errno;
        umask(old);
        errno = saved_errno;
        return -1;
    }

    // задаем 0600
    if (chmod(path, S_IRUSR | S_IWUSR) != 0) {
        saved_errno = errno;
        umask(old);
        errno = saved_errno;
        return -1;
    }
    umask(old);
    return 0;
}

static void try_fopen(const char *path, const char *stage) {
    print_ids(stage);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
    } else {
        printf("файл \"%s\" открыт для чтения. \n", path);
        fclose(fp);
    }
}

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : NULL;

    // гарантируем права
    if (make_file_private(path) != 0) {
        perror("make_file_private");
        return EXIT_FAILURE;
    }

    // печать + пытаемся открыт файл
    try_fopen(path, "до setuid");

    // ruid = euid
    if (seteuid(getuid()) != 0) {
        perror("seteuid(getuid())");
    }

    // повторяем
    try_fopen(path, "после seteuid");

    return EXIT_SUCCESS;

}
