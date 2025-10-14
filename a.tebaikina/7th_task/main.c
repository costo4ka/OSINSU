#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

volatile int timeout = 0;

void alarm_handler()
{
    printf("Время вышло\n");
    timeout = 1;
    fclose(stdin);
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл>\n", argv[0]);
        return -1;
    }

    signal(SIGALRM, alarm_handler);

    // открываем файл для чтения
    int file = open(argv[1], O_RDONLY);
    if (file == -1) {
        perror("Ошибка при открытии файла");
        return -1;
    }

    // получаем размер файла
    struct stat sb;
    if (fstat(file, &sb) == -1) {
        perror("Ошибка при получении информации о файле");
        close(file);
        return -1;
    }

    // отображаем файл в память
    char *mapped_file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, file, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Ошибка при отображении файла в память");
        close(file);
        return -1;
    }

    // анализируем файл
    off_t start[1024];
    int lens[1024];
    int len = 0, str_count = 0;

    for (off_t pos = 0; pos < sb.st_size; pos++) {
        if (len == 0)
            start[str_count] = pos;

        len += 1;

        if (mapped_file[pos] == '\n') {
            lens[str_count] = len;
            len = 0;
            str_count += 1;
        }
    }

    // если последняя строка без завершающего '\n'
    if (len > 0 && str_count < 1024) {
        lens[str_count] = len;
        str_count += 1;
    }

    // выводим информацию о строках
    printf("Line | Offset | Length\n");
    for (int i = 0; i < str_count; i++)
        printf("%d\t%lld\t%d\n", i + 1, (long long)start[i], lens[i] - 1);

    // запрос номера строки
    int n, f = 0;
    char rez[1025];

    printf("\nВведите номер строки\n");
    while (1) {
        if (f == 0)
            alarm(5);

        if (scanf("%d", &n) == 1 && n > 0 && n <= str_count) {
            if (f == 0) {
                alarm(0);
                f = 1;
                if (timeout == 1) {
                    break;
                }
            }

            const off_t st = start[n - 1];
            const int l = lens[n - 1];

            // копируем строку из отображенной памяти
            for (int i = 0; i < l; i++) {
                rez[i] = mapped_file[st + i];
            }
            rez[l] = '\0';

            printf("%s", rez);
        } else {
            if (timeout == 1 || n == 0)
                break;

            if (f == 0) {
                alarm(0);
                f = 1;
                if (timeout == 1) {
                    break;
                }
            }

            printf("Введите число - номер существующей строки\n");
            while (getchar() != '\n');
        }
    }

    // если время истекло, выводим все строки
    if (timeout == 1) {
        for (int i = 0; i < str_count; i++) {
            const off_t st = start[i];
            const int l = lens[i];

            for (int j = 0; j < l; j++) {
                rez[j] = mapped_file[st + j];
            }
            rez[l] = '\0';

            printf("%s", rez);
        }
    }

    // освобождаем ресурсы
    munmap(mapped_file, sb.st_size);
    close(file);

    return 0;
}