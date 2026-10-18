#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "generic.h"

// Вспомогательная функция для изменения размера
static bool needToResize(Vector *vector, bool *increase)
{
    if (vector->size >= vector->capacity) // Увеличиваем если размер достиг вместимости
    {
        *increase = true; // увеличиваем
        return true;
    }
    // Уменьшаем capacity если используется менее 25% (опционально)
    if (vector->size < vector->capacity / 4 && vector->capacity > MIN_SIZE)
    {
        *increase = false;  // уменьшаем
        return true;
    }
    return false;
}

// Определяем увеличивать размер или уменьшать
static int resize(Vector *vector, bool increase)
{
    size_t new_capacity; // новый размер вместимости
    if (increase) // увеличиваем
    {
        new_capacity = vector->capacity * 2; // Увеличиваем в 2 раза
    }
    else
    {
        new_capacity = vector->capacity / 2; // Уменьшаем в 2 раза
        if (new_capacity < MIN_SIZE) // не меньше минимального размера
            new_capacity = MIN_SIZE; // гарантируем минимум
    }

    void *new_data = realloc(vector->data, new_capacity * vector->elem_size); // изменяем размер блока данных на вместимость* размер элемента
    if (!new_data) // если не удалось выделить память
        return -1;

    vector->data = new_data; // обновляем указатель на данные
    vector->capacity = new_capacity; // обновляем вместимость
    return 0;
}

Vector *createVector(size_t elem_size)
{
    if (elem_size == 0) // некорректный размер элемента
        return NULL;

    Vector *vector = malloc(sizeof(Vector)); // выделяем память под структуру вектора
    if (!vector)
        return NULL;

    vector->data = malloc(elem_size * MIN_SIZE); // выделяем память под данные
    if (!vector->data)
    {
        free(vector); // освобождаем память под структуру вектора
        return NULL;
    }

    vector->elem_size = elem_size; // размер одного элемента
    vector->size = 0; // изначально нет элементов
    vector->capacity = MIN_SIZE; // начальная вместимость. гарантия минимума
    return vector;
}

int appendVectorItem(Vector *vector, void *el) // добавляет элемент в конец вектора
{
    if (!vector || !el) // O(1)
        return -1;

    bool increase;
    if (needToResize(vector, &increase)) // O(1)
    {
        if (increase) // O(1)
        {
            if (resize(vector, true) != 0) // O(n)
                return -1;
        }
    }

    // вычисляем указатель на место добавления нового элемента
    void *dest = (char *)vector->data + (vector->size * vector->elem_size); // O(1)
    // копируем данные в вектор
    memcpy(dest, el, vector->elem_size); // O(1)
    vector->size++; // O(1)
    return 0;
    // Оценка сверху - O(n) - при ресайзе
    // Точная оценка - Θ(n) - при ресайзе
    // Оценка снизу - Ω(1) - без ресайза
}

void *getVectorItem(Vector *vector, size_t index)
{
    if (vector == NULL || index >= vector->size) // O(1)
        return NULL;

    return (char *)vector->data + (index * vector->elem_size); // O(1)
    // Оценка сверху - O(1) - у нас прямой доступ к элементу по индексу
    // Точная оценка - Θ(1)
    // Оценка снизу - Ω(1)
}

int setVectorItem(Vector *vector, size_t index, void *value) // заменяет значение элемента по индексу
{
    if (!vector || !value || index >= vector->size) // O(1)
        return -1;

    void *dest = (char *)vector->data + (index * vector->elem_size); // O(1)
    memcpy(dest, value, vector->elem_size); // O(1)
    return 0;
    // Оценка сверху - O(1) - также имеем доступ к элементу нет никаких итераций
    // Точная оценка - Θ(1)
    // Оценка снизу - Ω(1)
}

void *popVectorItem(Vector *vector, size_t index)
{
    if (!vector || index >= vector->size) // O(1)
        return NULL;

    // Копируем элемент для возврата
    void *result = malloc(vector->elem_size); // O(1)
    if (!result)
        return NULL;

    void *src = (char *)vector->data + (index * vector->elem_size); // указатель на удаляемый элемент
    memcpy(result, src, vector->elem_size); // O(1)

    // Сдвигаем оставшиеся элементы влево
    if (index < vector->size - 1) // O(1)
    {
        void *next = (char *)vector->data + ((index + 1) * vector->elem_size); // O(1)
        memmove(src, next, (vector->size - index - 1) * vector->elem_size); // O(n-index-1)
    }

    vector->size--;

    // уменьшаем capacity если нужно
    bool increase;
    if (needToResize(vector, &increase) && !increase) // O(1)
    {
        resize(vector, false); // O(n)
    }

    return result; // O(1)
    // Оценка сверху - O(n) - если сдвигать пришлось все элементы
    // Точная оценка - Θ(n) - среднее
    // Оценка снизу - Ω(1) - если элемент в конце
}

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp) // возвращает индекс первого найденного элемента
{
    if (!vector || !value || vector->size == 0) // O(1)
        return -1;

    for (size_t i = 0; i < vector->size; i++) // O(n)
    {
        void *el = (char *)vector->data + (i * vector->elem_size); // O(1)
        bool equal;
        if (cmp)
            equal = cmp(el, value); // O(1)
        else
            equal = (memcmp(el, value, vector->elem_size) == 0); // O(1)

        if (equal)
            return (long int)i; // O(1)
    }

    return -1;
    // Оценка сверху - O(n) - проходимся по всему вектору
    // Точная оценка - Θ(n) - среднее
    // Оценка снизу - Ω(1) - если первый элемент
}

int vectorFree(Vector *vector) // освобождает память, занятую вектором
{
    if (!vector) // O(1)
        return -1;

    free(vector->data); // O(1)
    free(vector); // O(1)
    return 0;
    // Оценка сверху - O(1)
    // Точная оценка - Θ(1)
    // Оценка снизу - Ω(1)
}