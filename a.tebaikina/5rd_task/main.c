#include <stdio.h>
#include <fcntl.h>      // для работы с файловыми дескрипторами
#include <unistd.h>     // read, lseek, close


int main(int argc, const char * argv[])
{
    // открываем файл
    const int file = open(argv[1], O_RDONLY);
    if (file == -1)
        return -1;

    off_t start[1024];  // хранение смещений строк в файле
    int lens[1024];     // хранение длин строк
    char val;           // переменная для чтения одного символа
    int len = 0;        // текущая длина строки
    int str_count = 0;  // счетчик строк
    off_t pos = 0;      // текущая позиция в файле

    // читаем посимвольно
    while (read(file, &val, 1) == 1)
    {
        if (len == 0)  // если это начало новой строки
            start[str_count] = pos;  // запоминаем смещение начала строки

        len += 1;  // увеличиваем длину текущей строки
        pos += 1;  // увеличиваем счетчик позиции в файле

        if (val == '\n')  // если встретили символ новой строки
        {
            lens[str_count] = len;  // сохраняем длину строки
            len = 0;                // сбрасываем длину для следующей строки
            str_count += 1;         // увеличиваем счетчик строк
        }
    }
    // проверяем, была ли последняя строка без завершающего '\n'
    if (len > 0)
    {
        lens[str_count] = len;  // сохраняем длину последней строки
        str_count += 1;         // увеличиваем счетчик строк
    }

    // вывод
    printf("Line Offset length\n");
    for (int i = 0; i < str_count; i++)
        printf("%d\t%lld\t%d\n", i + 1, (long long)start[i], lens[i] - 2);  // lens[i] - 1, чтобы исключить '\n'

    int n;
    char rez[1024]; // буфер для хранения строки

    // заправшиваме номер
    while (1)
    {
        if (scanf("%d", &n) == 1 && n > 0 && n < str_count + 1)
        {
            if (n > str_count || n < 0)
                return -1;

            // получаем смещение и длину строки из ранее сохраненных массивов
            const off_t st = start[n - 1];
            const int l = lens[n - 1];

            // перемещаем указатель чтения в файле на начало нужной строки
            lseek(file, st, SEEK_SET);

            read(file, rez, l);

            rez[l] = '\0';

            printf("%s", rez);
        }
        else
        {
            // если ввод некорректен
            printf("Введите число - номер существующей строки\n");

            while (getchar() != '\n');
        }

        // если пользователь ввел 0, завершаем программу
        if (n == 0)
            break;
    }


    close(file);

    return 0;
}