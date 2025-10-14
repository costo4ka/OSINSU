#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/mman.h>

#define MAX_LINES 1024

sig_atomic_t timed_out = 0;

void handler(int sig) {
    (void)sig;
    timed_out = 1;
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t filesize = st.st_size;

    /*
     * mmap создаёт отображение файла в адресное пространство процесса.
     * Параметры:
     *  - первый аргумент NULL: система сама выберет адрес для отображения
     *  - filesize: сколько байт отобразить
     *  - PROT_READ: разрешаем только чтение
     *  - MAP_PRIVATE: создаём частное отображение
     *  - fd: файловый дескриптор, который отображаем
     *  - 0: смещение от начала файла
     */
    char *data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    off_t start[MAX_LINES];
    int lens[MAX_LINES];
    int len = 0, str_count = 0;

    for (size_t pos = 0; pos < filesize; pos++) {
        if (len == 0)
            start[str_count] = pos;

        len += 1;

        if (data[pos] == '\n') {
            lens[str_count] = len;
            len = 0;
            str_count += 1;
            if (str_count >= MAX_LINES) break;
        }
    }

    printf("Line\tOffset\tLength\n");
    for (int i = 0; i < str_count; i++)
        printf("%d\t%lld\t%d\n", i + 1, (long long)start[i], lens[i] - 1);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");

         // при ошибке нужно обязательно освободить отображённую память с помощью munmap

        munmap(data, filesize);
        close(fd);
        return 1;
    } // ← добавлена недостающая закрывающая скобка

    int n;
    char rez[1024];

    while (1) {
        printf("Введите номер строки (0 — выход): ");
        fflush(stdout);

        timed_out = 0;
        alarm(5);

        if (scanf("%d", &n) != 1 || timed_out) {
            if (timed_out) {
                printf("\nВремя вышло! Содержимое файла:\n\n");
                write(STDOUT_FILENO, data, filesize); // напрямую выводим
            } else {
                printf("Введите число - номер существующей строки\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
            }
            break;
        }

        alarm(0);

        if (n == 0)
            break;

        if (n > 0 && n <= str_count) {
            const off_t st = start[n - 1];
            const int l = lens[n - 1];
            //копируем нужный фрагмент байт с помощью memcpy(data + st — это адрес начала нужной строки в файле)
            memcpy(rez, data + st, l);
            rez[l] = '\0';
            printf("%s", rez);
        } else {
            printf("Введите число - номер существующей строки\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    munmap(data, filesize);
    close(fd);
    return 0;
}
