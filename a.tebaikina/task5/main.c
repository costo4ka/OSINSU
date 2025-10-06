#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> // file control
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_LINES 1024   // максимум строк
#define BUF_SIZE  4096   // буфер чтения

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    off_t offsets[MAX_LINES]; // массив, с какого байта начинается каждая строка
    size_t lens[MAX_LINES]; // массив, сколько символов в каждой строке
    int line_count = 0; // сколько строк мы уже нашли

    // первая строка - всегда нулевой байт
    offsets[0] = 0;
    line_count = 1;

    char buffer[BUF_SIZE]; // временный буфер для чтения из файла блоками
    ssize_t n;            // сколько реально байт прочитал read()
    off_t pos = 0;        // "где мы находимся" в файле (общая позиция)
    off_t next_line_start = 0; // откуда началась текущая строка


    while ((n = read(fd, buffer, BUF_SIZE)) > 0) {
        for (size_t i = 0; i < n; i++) {
            if (buffer[i] == '\n') {
                off_t line_end = pos + i;
                lens[line_count - 1] = (line_end - next_line_start) + 1; // включая '\n'
                next_line_start = line_end + 1;

                if (line_count < MAX_LINES) {
                    offsets[line_count - 1] = next_line_start;
                }
                line_count++;
            }
        }
        pos += n;
    }

    // еслм строка не закончилась \n
    if (next_line_start < pos) {
        // от размера файла вычитаем начало строки - получаем лину послденей строки без \n
        lens[line_count - 1] = pos - next_line_start;
    }

    if (n < 0) {
        perror("read");
        close (fd);
        return 1;
    }

    printf(" таблица строк: \n");
    for (int i = 0; i < line_count; i++) {
        printf("Строка %d: offset=%ld length=%zu\n",
               i+1, (long)offsets[i], lens[i]);
    }


    while (1) {
        int num;
        printf("Введите номер строки  !(0 = выход)!: ");
        if (scanf("%d", &num) != 1) break;
        if (num == 0) break;

        if (num < 1 || num > line_count) {
            printf("Нет такой строки!\n");
            continue;
        }
        // "перемотка" файла: устанавливает указатель на позицию в файле
        // offset — на сколько сместиться,
        // whence — откуда считать (SEEK_SET — от начала, SEEK_CUR — от текущей, SEEK_END — от конца)
        if (lseek(fd, offsets[num-1], SEEK_SET) < 0) {
            perror("lseek");
            continue;
        }

        char *linebuf = malloc(lens[num-1] + 1);
        if (!linebuf) {
            perror("malloc");
            break;
        }

        ssize_t r = read(fd, linebuf, lens[num-1]);
        if (r < 0) {
            perror("read");
            free(linebuf);
            continue;
        }

        linebuf[lens[num-1]] = '\0';
        printf("Строка %d: %s", num, linebuf);

        free(linebuf);
    }

    close(fd);
    return 0;
}

