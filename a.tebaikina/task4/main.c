#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

typedef struct Node{
    char *text;
    struct Node *next;
} Node;

int main(void) {
    char buffer[BUFFER_SIZE];

    Node *head = NULL;
    Node *tail = NULL;

    while (1) {
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // убираем '\n'
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            --len;
        }

        // точка на отдельной строке — выход
        if (len == 1 && buffer[0] == '.') {
            break;
        }

        char *copy = malloc(len + 1);
        if (!copy) {
            fprintf(stderr, "не удалось выделить память для строки\n");
            // очистка списка перед выходом
            for (Node *p = head; p; ) {
                Node *n = p->next;
                free(p->text);
                free(p);
                p = n;
            }
            return 1;
        }
        memcpy(copy, buffer, len + 1);

        Node *node = malloc(sizeof(Node));
        if (!node) {
            fprintf(stderr, "не удалось выделить память для узла списка\n");
            free(copy);
            for (Node *p = head; p; ) {
                Node *n = p->next;
                free(p->text);
                free(p);
                p = n;
            }
            return 1;
        }
        node->text = copy;
        node->next = NULL;

        if (!head) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    // выводим все строки
    for (Node *p = head; p; p = p->next) {
        puts(p->text);
    }

    // освобождаем память
    for (Node *p = head; p; ) {
        Node *n = p->next;
        free(p->text);
        free(p);
        p = n;
    }
    return 0;
}
