#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

static void print_ids(const char *stage) {
    uid_t r = getuid();
    uid_t e = geteuid();
    printf("[%s] ruid=%ld euid=%ld\n", stage, (long)r, (long)e);
}


static void try_fopen(const char *path, const char *stage) {
    print_ids(stage);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
    } else {
        printf("Файл \"%s\" успешно открыт для чтения.\n", path);
        fclose(fp);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[1];

    // 1. Печатаем реальные и эффективные UID, пробуем открыть файл
    try_fopen(path, "до setuid");

    /*
     * 2. Здесь используем setuid(geteuid()), чтобы real == effective == владелец файла.
     */
    if (setuid(geteuid()) != 0) {
        perror("setuid(geteuid())");
    }

    // 3. Повторяем шаги: печатаем UID и пытаемся открыть файл
    try_fopen(path, "после setuid");

    return EXIT_SUCCESS;
}

