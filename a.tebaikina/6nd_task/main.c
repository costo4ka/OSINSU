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

int main(int argc, const char * argv[])
{
    signal(SIGALRM, alarm_handler);

    const int file = open(argv[1], O_RDONLY);
    if (file == -1)
        return -1;

    off_t start[1024], pos = 0;
    int lens[1024];
    char val;
    int len = 0, str_count = 0;

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
        }
    }

    if (len > 0 && str_count < 1024)
    {
        lens[str_count] = len;
        str_count += 1;
    }

    printf("Line | Offset | Length\n");
    for (int i = 0; i < str_count; i++)
        printf("%d\t%lld\t%d\n", i + 1, (long long)start[i], lens[i] - 1);


    int n, f = 0;
    char rez[1025];

    printf("\nВведите номер строки\n");
    while (1)
    {
        if (f == 0)
            alarm(5);
        if (scanf("%d", &n) == 1 && n > 0 && n < str_count + 1)
        {
            if (f == 0)
            {
                alarm(0);
                f = 1;
                if (timeout == 1)
                {
                    break;
                }
            }

            if (n > str_count || n < 0)
                return -1;

            const off_t st = start[n - 1];
            const int l = lens[n - 1];

            lseek(file, st, SEEK_SET);
            read(file, rez, l);
            rez[l] = '\0';
            printf("%s", rez);
        }
        else
        {
            if (timeout == 1 || n == 0)
                break;

            if (f == 0)
            {
                alarm(0);
                f = 1;
                if (timeout == 1)
                {
                    break;
                }
            }

            printf("Введите число - номер существующей строки\n");
            while (getchar() != '\n');
        }
    }

    if (timeout == 1)
    {
        for (int i = 0; i < str_count; i++) {
            lseek(file, start[i], SEEK_SET);
            read(file, rez, lens[i]);
            rez[lens[i]] = '\0';
            printf("%s", rez);
        }
    }

    close(file);

    return 0;
}