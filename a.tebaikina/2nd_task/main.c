#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main(void) {
    time_t now;
    struct tm *sp;

    // устанавливаем PST
    setenv("TZ", "PST8PDT", 1);
    tzset();

    time(&now);

    sp = localtime(&now);
    printf("California: %d/%d/%d %d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,   /* прибавляем 1900 для корректного года */
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]);

    return 0;
}