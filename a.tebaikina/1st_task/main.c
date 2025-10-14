#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // getopt, getpid
#include <sys/resource.h> // getrlimit, setrlimit
#include <errno.h>
#include <string.h>

extern char **environ;  // глобальный массив переменных окружения

int main(int argc, char *argv[]) {
    int opt;
    struct rlimit lim;
    char cwd[256];             // буфер для текущей директории

    // массивы, чтобы сохранить найденные опции и их аргументы
    char opts[argc];
    char *args[argc];
    int count = 0;             // сколько всего опций найдено

    // разбор аргументов командной строки
    // getopt возвращает одну букву опции ( 'i' для -i)
    // optarg содержит значение, если опция требует аргумент ( -U 100)
    while ((opt = getopt(argc, argv, "ispucCdvU:V:")) != -1) {
        opts[count] = opt;     // сохраняем букву опции
        args[count] = optarg;  // сохраняем возможный аргумент
        count++;               // увеличиваем счётчик опций
    }

    // Выполняем опции в порядке справа налево
    for (int i = count - 1; i >= 0; i--) {
        switch (opts[i]) {

            // --- -i : показать идентификаторы пользователя и группы ---
            case 'i':
                printf("Real UID: %d, GID: %d\n", getuid(), getgid());
                printf("Effective UID: %d, GID: %d\n", geteuid(), getegid());
                break;

            // --- -s : сделать процесс лидером группы ---
            case 's':
                if (setpgid(0, 0) == 0)
                    printf("Process became the group leader\n");
                else
                    perror("setpgid");  // если не получилось
                break;

            // --- -p : вывести PID, PPID и PGID ---
            case 'p':
                printf("PID: %d, PPID: %d, PGID: %d\n",
                       getpid(), getppid(), getpgrp());
                break;

            // --- -u : показать лимит открытых файлов (ulimit -n) ---
            case 'u':
                if (getrlimit(RLIMIT_NOFILE, &lim) == 0)
                    printf("Ulimit (files): soft=%llu, hard=%llu\n",
                           (unsigned long long)lim.rlim_cur,
                           (unsigned long long)lim.rlim_max);
                else
                    perror("getrlimit"); // если ошибка
                break;

            // --- -U <value> : изменить лимит открытых файлов ---
            case 'U': {
                errno = 0;
                long val = strtol(args[i], NULL, 10); // преобразуем строку в число
                if (errno != 0) {                // если ошибка преобразования
                    perror("Invalid number for -U");
                    break;
                }
                if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
                    lim.rlim_cur = val;          // новое значение лимита
                    if (setrlimit(RLIMIT_NOFILE, &lim) == 0)
                        printf("Changed ulimit to %ld\n", val);
                    else
                        perror("setrlimit");     // если не удалось изменить
                }
                break;
            }

            // --- -c : показать размер core-файлов ---
            case 'c':
                if (getrlimit(RLIMIT_CORE, &lim) == 0)
                    printf("Core file size: soft=%llu, hard=%llu\n",
                           (unsigned long long)lim.rlim_cur,
                           (unsigned long long)lim.rlim_max);
                else
                    perror("getrlimit");
                break;

            // --- -C <value> : изменить размер core-файлов ---
            case 'C': {
                errno = 0;
                long val = strtol(args[i], NULL, 10);
                if (errno != 0) {
                    perror("Invalid number for -C");
                    break;
                }
                if (getrlimit(RLIMIT_CORE, &lim) == 0) {
                    lim.rlim_cur = val;
                    if (setrlimit(RLIMIT_CORE, &lim) == 0)
                        printf("Changed core size to %ld\n", val);
                    else
                        perror("setrlimit");
                }
                break;
            }

            // --- -d : показать текущую рабочую директорию ---
            case 'd':
                if (getcwd(cwd, sizeof(cwd)))       // getcwd возвращает путь
                    printf("Current directory: %s\n", cwd);
                else
                    perror("getcwd");               // если не удалось получить
                break;

            // --- -v : распечатать все переменные окружения ---
            case 'v': {
                for (char **env = environ; *env; env++) // перебираем все строки окружения
                    printf("%s\n", *env);
                break;
            }

            // --- -VNAME=value : добавить/изменить переменную окружения ---
            case 'V': {
                char *eq = strchr(args[i], '=');   // ищем знак "="
                if (!eq) {
                    fprintf(stderr, "Usage: -VNAME=value\n");
                    break;
                }
                *eq = '\0';                        // отделяем имя от значения
                if (setenv(args[i], eq + 1, 1) == 0)
                    printf("Set variable %s=%s\n", args[i], eq + 1);
                else
                    perror("setenv");              // если ошибка
                break;
            }

            // --- неизвестная опция ---
            case '?':
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                break;
        }
    }

    return 0; // конец программы
}
