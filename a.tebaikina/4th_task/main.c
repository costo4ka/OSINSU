#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 4096  // максимальный размер строки, которую мы можем прочитать

typedef struct Node {
    char *text;          // строка, которую мы сохраним
    struct Node *next;   // указатель на следующий элемент списка
} Node;

// состоит ли строка только из букв
int is_alpha_string(const char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!isalpha(c) && c != '\0') {  // допускаем только буквы
            return 0;
        }
    }
    return 1;
}

int main(void) {
    char buffer[BUFFER_SIZE]; // временный буфер для чтения строк

    Node *head = NULL; // начало списка
    Node *tail = NULL; // конец списка

    // основной цикл чтения строк
    while (1) {
        // читаем строку из стандартного ввода
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // убираем символ новой строки '\n' в конце, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            --len;
        }

        // если введена точка на отдельной строке — выходим из цикла
        if (len == 1 && buffer[0] == '.') {
            break;
        }

        if (!is_alpha_string(buffer)) {
            fprintf(stderr, " Строка \"%s\" содержит недопустимые символы — пропускаем.\n", buffer);
            continue;
        }

        // выделяем память под копию строки
        char *copy = malloc(len + 1);
        if (!copy) {
            fprintf(stderr, "не удалось выделить память для строки\n");

            // если ошибка — очищаем уже созданный список и выходим
            for (Node *p = head; p;) {
                Node *n = p->next;
                free(p->text); // освобождаем строку
                free(p); // освобождаем узел
                p = n; // двигаемся дальше
            }
            return 1;
        }

        // копируем строку из буфера в новую память
        memcpy(copy, buffer, len + 1);

        // создаем новый узел списка
        Node *node = malloc(sizeof(Node));
        if (!node) {
            fprintf(stderr, "не удалось выделить память для узла списка\n");
            free(copy); // освобождаем память строки

            // очищаем список
            for (Node *p = head; p;) {
                Node *n = p->next;
                free(p->text);
                free(p);
                p = n;
            }
            return 1;
        }

        node->text = copy;  // сохраняем строку в узел
        node->next = NULL;  // следующий элемент пока неизвестен

        // добавляем узел в конец списка
        if (!head) {
            // если это первый элемент — он и head, и tail
            head = node;
            tail = node;
        } else {
            // если список уже есть — цепляем к хвосту
            tail->next = node;
            tail = node;
        }
    }

    // выводим все строки, которые сохранили
    for (Node *p = head; p; p = p->next) {
        puts(p->text);
    }

    // освобождаем всю выделенную память
    for (Node *p = head; p;) {
        Node *n = p->next;
        free(p->text); // освобождаем строку
        free(p);       // освобождаем сам узел
        p = n;
    }

    return 0;
}
