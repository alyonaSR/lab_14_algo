#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GenericList *createList(size_t elem_size)
{
    GenericList *list = malloc(sizeof(GenericList));
    if (list != NULL) { // проверяем выделилась ли вообще память
        list->head = NULL; // делаем первое значение списка NULL (пустая голова)
        list->elem_size = elem_size; // придаем списку размер
    }
    else {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    } return list;
}

void appendItem(GenericList *list, void *data)
{
    if (list == NULL || data == NULL) { // O(1)
        return;
    }
    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL ) { // O(1) всего лишь проверка
        return;
    }
    new_node->data = malloc(list->elem_size);
    if (new_node->data == NULL) { // O(1) проверка
        free(new_node);           // О(1) освобождаем
        return;
    }
    memcpy(new_node->data, data, list->elem_size); // O(1)
    new_node->next = NULL;      // O(1)
    if (list->head == NULL) {   // O(1)
        list->head = new_node;      // O(1)
    } else {
        Node *current = list->head; // O(1)
        while (current->next != NULL) { // O(n) - цикл зависит от количества нуллевых элементов
            current = current->next;    // О(1) - от одного т к внутри цикла
        }
        current->next = new_node; // O(1) - присваиваем
    }
    // Оценка сверху - O(n) - проходимся по всем элементам
    // Точная оценка - Θ(n) - n/2 с округлением
    // Оценка снизу - Ω(1) - если список окажется пуст всего одна проходка
}

int findItem(GenericList *list, void *value, EqualsFunc cmp)
{
    if (list == NULL || value == NULL || cmp == NULL) // O(1) - проверка
        return -1;

    Node *cur = list->head; // O(1)
    int index = 0;          // O(1)

    while (cur) // O(n) n = количество элементов
    {
        if (cmp(cur->data, value)) // O(1)
            return index;          // O(1)

        cur = cur->next; // O(1)
        index++;         // O(1)
    }

    return -1; // O(1)
    // Оценка сверху - O(n) - весь проход цикла
    // Точная оценка - Θ(n) - в среднем n/2
    // Оценка снизу - Ω(1) - проход если элемент в начале
}

void *popItem(GenericList *list, size_t index)
{
    if (list == NULL || list->head == NULL) { // O(1)
        return NULL;
    }
    Node *current = list->head;                 // O(1)
    Node *previous = NULL;                      // O(1)
    int current_index = 0;                      // O(1)
    while (current && current_index < index) {  // O(i) i = index
        previous = current;                     // O(1)
        current = current->next;                // O(1)
        current_index++;
    }
    if (current == NULL) { // O(1)
        return NULL;
    }
    if (previous) { // O(n) n = количество узлов
        previous->next = current->next; // O(1)
    } else {
        list->head = current->next;     // O(1)
    }
    void *data_copy = malloc(list->elem_size); // O(1)
    if (data_copy) { // O(1) - проверка
        memcpy(data_copy, current->data, list->elem_size); // O()
    }
    free(current->data); // O(1)
    free(current);       // O(1)
    return data_copy;    // O(1)

    // Оценка сверху - O(n) - если элемент окажется в конце
    // Точная оценка - Θ(n) - среднее
    // Оценка снизу - Ω(1) - если элемент в начале списка
}

void freeList(GenericList *list)
{
    if (list == NULL) { // O(1)
        return; // O(1)
    }
    Node *current = list->head; // O(1)
    while (current) { // O(n) все элемента
        Node *next = current->next; // O(1)
        free(current->data);    // O(1)
        free(current);          // O(1)
        current = next;         // O(1)
    }
    return free(list); // O(1)
    // Оценка сверху - O(n) - весь список освобождаем
    // Точная оценка - Θ(n) - средняя
    // Оценка снизу - Ω(1) - если список уже пуст одна проходка
}

unsigned int listLength(GenericList *list)
{
    if (list == NULL) {
        return 0; // O(1)
    }
    size_t count = 0;
    Node *current = list->head; // O(1)
    while (current) { // O(n)
        count ++;     // O(1) в цикле
        current = current->next; // O(1)
    }
    return count; // О(1)
    // Оценка сверху - O(n) - все элементы
    // Точная оценка - Θ(n) - все элементы
    // Оценка снизу - Ω(1) - если список пуст
}

// функция нахождения значения по ключу
void *getItem(GenericList *list, size_t index)
{
    if (list == NULL || list->head == NULL) { // O(1)
        return NULL;
    }
    Node *current = list->head;                 // O(1)
    size_t current_index = 0;                   // O(1)
    while (current && current_index < index) {  // O(i) i = index
        current = current->next;                // O(1)
        current_index++;                        // O(1)
    }
    if (current == NULL) { // O(1)
        return NULL;
    }
    return current->data; // O(1)
    // Оценка сверху - O(n) - если индекс в конце
    // Точная оценка - Θ(n) - среднее n/2
    // Оценка снизу - Ω(1) - если нулевой индекс
}