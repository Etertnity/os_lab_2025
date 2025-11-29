// Задание 1: Последовательный поиск минимума и максимума
// Демонстрация работы функции GetMinMax.
// Используются аргументы командной строки и динамическая память.

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed array_size\n", argv[0]);
        return 1;
    }

    // seed — для генерации случайного массива
    int seed = atoi(argv[1]);
    int array_size = atoi(argv[2]);

    // Выделяем память под массив
    int *array = malloc(array_size * sizeof(int));

    // Заполняем массив случайными числами
    GenerateArray(array, array_size, seed);

    // Вызываем функцию поиска минимума и максимума
    struct MinMax min_max = GetMinMax(array, 0, array_size);

    // Освобождаем память
    free(array);

    // Выводим результат
    printf("min: %d\n", min_max.min);
    printf("max: %d\n", min_max.max);

    return 0;
}