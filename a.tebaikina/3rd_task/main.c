#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// печатает реальные и эффективные идентификаторы пользователя
static void print_ids(const char *stage) {
    uid_t r = getuid();
    uid_t e = geteuid();
    printf("[%s] ruid=%ld euid=%ld\n", stage, (long)r, (long)e);
}

// пытается открыть файл и сообщает результат
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
    const char *path = (argc > 1) ? argv[1] : NULL;

    if (!path) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 1. печатаем реальные и эффективные UID
    // 2. пробуем открыть файл (fopen)
    try_fopen(path, "до setuid");

    // 3. делаем реальные и эффективные идентификаторы одинаковыми
    if (setuid(getuid()) != 0) {
        perror("setuid(getuid())");
    }

    // 4. повторяем первые два шага
    try_fopen(path, "после setuid");

    return EXIT_SUCCESS;
}

//  echo "секретные данные" > secret.txt
//  chmod 600 secret.txt
//  gcc main.c -o main
//  ./main secret.txt