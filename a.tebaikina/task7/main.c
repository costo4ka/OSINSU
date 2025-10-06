#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>    // mmap — отображает файл в память; munmap — освобождает память, когда программа завершает работу
#include <signal.h>      // добавлено для alarm() и сигналов
#include <string.h>      // добавлено для memset и sigemptyset

#define MAX_LINES 1024
#define BUF_SIZE  4096

volatile sig_atomic_t timed_out = 0;
void handler(int sig) { (void)sig; timed_out = 1; } // обработчик сигнала SIGALRM

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    fstat(fd, &st);
    size_t filesize = st.st_size;                 // получаем размер файла для mmap

    // отображаем файл в память
    char *data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    off_t offsets[MAX_LINES];
    size_t lens[MAX_LINES];
    int line_count = 0;

    offsets[0] = 0;
    line_count = 1;

    // вычисляем границы строк прямо в памяти
    for (size_t i = 0; i < filesize; i++) {
        if (data[i] == '\n') {
            lens[line_count - 1] = (i - offsets[line_count-1]) + 1;
            if (line_count < MAX_LINES) offsets[line_count] = i + 1;
            line_count++;
        }
    }
    if (offsets[line_count-1] < filesize)
        lens[line_count-1] = filesize - offsets[line_count-1];

    printf(" таблица строк: \n");
    for (int i = 0; i < line_count; i++) {
        printf("Строка %d: offset=%ld length=%zu\n",
               i+1, (long)offsets[i], lens[i]);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        munmap(data, filesize);
        close(fd);
        return 1;
    }

    while (1) {
        int num;
        printf("Введите номер строки !(0=выход)!: ");
        fflush(stdout);

        timed_out = 0;                            // сбрасываем флаг таймаута перед вводом
        alarm(5);                                 // даём пользователю 5 секунд на ввод

        // если время вышло или ввод не удался
        if (scanf("%d", &num) != 1 || timed_out) {
            printf("\nВремя вышло! Содержимое файла:\n\n");
            write(1, data, filesize);             // выводим весь файл
            break;
        }

        alarm(0);                                 // сбрасываем таймер, если ввод успешен

        if (num == 0) break;
        if (num < 1 || num > line_count) {
            printf("Нет такой строки!\n");
            continue;
        }

        // вместо lseek/read читаем напрямую из памяти
        write(1, data + offsets[num-1], lens[num-1]);
    }

    munmap(data, filesize);                       // освобождаем отображённую память
    close(fd);
    return 0;
}
