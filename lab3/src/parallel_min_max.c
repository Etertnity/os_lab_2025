// Задание 2: Параллельный поиск минимума и максимума
// Используем процессы (fork), IPC (pipe или файлы), wait и разбиение массива.

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool by_files = false;

    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);
        if (c == -1)
            break;

        if (c == 0) {
            if (option_index == 0) seed = atoi(optarg);
            else if (option_index == 1) array_size = atoi(optarg);
            else if (option_index == 2) pnum = atoi(optarg);
            else if (option_index == 3) by_files = true;
        }
    }

    // Проверяем корректность данных
    if (seed <= 0 || array_size <= 0 || pnum <= 0) {
        printf("Usage: --seed NUM --array_size NUM --pnum NUM [--by_files]\n");
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Массив pipe-ов
    int (*pipefds)[2] = NULL;

    if (!by_files) {
        // Выделяем pnum pipe-ов
        pipefds = malloc(sizeof(int[2]) * pnum);
    }

    // Разбиваем массив на части
    int chunk = array_size / pnum;
    int remainder = array_size % pnum;

    for (int i = 0; i < pnum; i++) {

        // Создаём pipe
        if (!by_files)
            pipe(pipefds[i]);

        pid_t pid = fork();

        if (pid == 0) {

            // Вычисляем границы участка массива
            int begin = i * chunk + (i < remainder ? i : remainder);
            int end = begin + chunk + (i < remainder ? 1 : 0);

            // Считаем min/max на участке
            struct MinMax local = GetMinMax(array, begin, end);

            if (by_files) {
                FILE *f = fopen("min_max.txt", "a");
                fprintf(f, "%d %d\n", local.min, local.max);
                fclose(f);
            } else {
                // Пишем структуру в pipe
                close(pipefds[i][0]); // закрываем конец чтения
                write(pipefds[i][1], &local, sizeof(struct MinMax));
                close(pipefds[i][1]);
            }

            exit(0);
        }
    }

    for (int i = 0; i < pnum; i++)
        wait(NULL);

    // Сводим результаты
    struct MinMax global;
    global.min = INT_MAX;
    global.max = INT_MIN;

    if (by_files) {
        FILE *f = fopen("min_max.txt", "r");
        int mn, mx;
        while (fscanf(f, "%d %d", &mn, &mx) == 2) {
            if (mn < global.min) global.min = mn;
            if (mx > global.max) global.max = mx;
        }
        fclose(f);
    } else {
        // Чтение из pipe-ов
        for (int i = 0; i < pnum; i++) {
            close(pipefds[i][1]);
            struct MinMax local;
            read(pipefds[i][0], &local, sizeof(local));
            close(pipefds[i][0]);

            if (local.min < global.min) global.min = local.min;
            if (local.max > global.max) global.max = local.max;
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    printf("Min: %d\nMax: %d\nTime: %f ms\n", global.min, global.max, elapsed);

    free(array);
    if (!by_files) free(pipefds);

    return 0;
}