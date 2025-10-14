#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_LINES 1024

sig_atomic_t timed_out = 0;

void handler(int sig) {
    (void)sig;
    timed_out = 1;
}

int main(int argc, const char * argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const int file = open(argv[1], O_RDONLY);
    if (file == -1) {
        perror("open");
        return 1;
    }

    off_t start[MAX_LINES];
    int lens[MAX_LINES];
    char val;
    int len = 0, str_count = 0;
    off_t pos = 0;

    while (read(file, &val, 1) == 1)
    {
        if (len == 0)
            start[str_count] = pos;

        len += 1;
        pos += 1;

        if (val == '\n')
        {
            lens[str_count] = len;
            len = 0;
            str_count += 1;
            if (str_count >= MAX_LINES) break; // защита от переполнения
        }
    }


    printf("Line\tOffset\tLength\n");
    for (int i = 0; i < str_count; i++)
        printf("%d\t%lld\t%d\n", i + 1, (long long)start[i], lens[i] - 2);

    // Настройка сигнала
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        close(file);
        return 1;
    }

    int n;
    char rez[1024];

    while (1)
    {
        printf("Введите номер строки (0 — выход): ");
        fflush(stdout);

        timed_out = 0;
        alarm(5); // 5 секунд на ввод

        if (scanf("%d", &n) != 1 || timed_out)
        {
            if (timed_out) {
                printf("\nВремя вышло! Содержимое файла:\n\n");
                lseek(file, 0, SEEK_SET);
                char buf[4096];
                ssize_t bytes;
                while ((bytes = read(file, buf, sizeof(buf))) > 0) {
                    write(STDOUT_FILENO, buf, bytes);
                }
            } else {
                printf("Введите число - номер существующей строки\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
            }
            break;
        }

        alarm(0); // отключаем таймер

        if (n == 0)
            break;

        if (n > 0 && n <= str_count)
        {
            const off_t st = start[n - 1];
            const int l = lens[n - 1];

            lseek(file, st, SEEK_SET);
            read(file, rez, l);
            rez[l] = '\0';
            printf("%s", rez);
        }
        else
        {
            printf("Введите число - номер существующей строки\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    close(file);
    return 0;
}