// Задание 1: Реализация функции поиска минимума и максимума
// Функция получает диапазон массива [begin, end) и должна вернуть MinMax.

#include "find_min_max.h"
#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
    // Инициализируем min и max крайними значениями
    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    // Перебираем элементы в указанном диапазоне
    for (unsigned int i = begin; i < end; i++) {
        if (array[i] < min_max.min)
            min_max.min = array[i];

        if (array[i] > min_max.max)
            min_max.max = array[i];
    }

    return min_max;
}
