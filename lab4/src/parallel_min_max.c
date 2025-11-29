// parallel_min_max.c
// ЛР3 + ЛР4: параллельный поиск min/max + таймаут с помощью alarm/kill

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
#include <signal.h>   // signal, kill, SIGALRM, SIGKILL

#include "find_min_max.h"
#include "utils.h"

// Глобальный флаг, который выставляется обработчиком SIGALRM
// sig_atomic_t — тип, безопасный для работы в signal handler
volatile sig_atomic_t timeout_expired = 0;

// Обработчик сигнала SIGALRM
void alarm_handler(int signum) {
    // Просто отмечаем, что время вышло
    timeout_expired = 1;
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool by_files = false;

    int timeout = 0; // таймаут в секундах, 0 = нет таймаута

    // --- Парсим аргументы командной строки ---
    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 0},
            {"timeout", required_argument, 0, 0}, // новый параметр
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        if (c == 0) {
            switch (option_index) {
            case 0:
                seed = atoi(optarg);
                break;
            case 1:
                array_size = atoi(optarg);
                break;
            case 2:
                pnum = atoi(optarg);
                break;
            case 3:
                by_files = true;
                break;
            case 4:
                // Новый параметр --timeout N
                timeout = atoi(optarg);
                if (timeout < 0) timeout = 0;
                break;
            default:
                fprintf(stderr, "Unknown option index %d\n", option_index);
                return 1;
            }
        }
    }

    if (seed <= 0 || array_size <= 0 || pnum <= 0) {
        fprintf(stderr, "Usage: %s --seed N --array_size N --pnum N [--by_files] [--timeout N]\n", argv[0]);
        return 1;
    }

    if (pnum > array_size)
        pnum = array_size;

    int *array = malloc(sizeof(int) * array_size);
    if (!array) {
        fprintf(stderr, "Cannot allocate memory\n");
        return 1;
    }

    GenerateArray(array, array_size, seed);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    struct MinMax global_min_max;
    global_min_max.min = INT_MAX;
    global_min_max.max = INT_MIN;

    // Массив PID-ов дочерних процессов (нужен, чтобы потом их убить, если таймаут)
    pid_t *child_pids = malloc(sizeof(pid_t) * pnum);
    if (!child_pids) {
        fprintf(stderr, "Cannot allocate memory for pids\n");
        free(array);
        return 1;
    }

    // Для варианта с pipe
    int (*pipefd)[2] = NULL;
    if (!by_files) {
        pipefd = malloc(sizeof(int[2]) * pnum);
        if (!pipefd) {
            fprintf(stderr, "Cannot allocate memory for pipes\n");
            free(array);
            free(child_pids);
            return 1;
        }
    }

    const char *filename = "min_max.txt";
    if (by_files) {
        // Очистим файл перед использованием
        FILE *f = fopen(filename, "w");
        if (!f) {
            fprintf(stderr, "Cannot open file for writing\n");
            free(array);
            free(child_pids);
            return 1;
        }
        fclose(f);
    }

    // Разбиваем массив на pnum частей (равномерно + остаток)
    int chunk_size = array_size / pnum;
    int remainder = array_size % pnum;

    for (int i = 0; i < pnum; i++) {
        // Вычисляем границы диапазона [begin, end)
        int begin = i * chunk_size + (i < remainder ? i : remainder);
        int end = begin + chunk_size + (i < remainder ? 1 : 0);

        if (!by_files) {
            // Создаём пайп для передачи результата от i-го ребёнка
            if (pipe(pipefd[i]) == -1) {
                perror("pipe");
                free(array);
                free(child_pids);
                free(pipefd);
                return 1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(array);
            free(child_pids);
            if (!by_files) free(pipefd);
            return 1;
        }

        if (pid == 0) {
            // --- Дочерний процесс ---

            // Считаем локальный min/max на своём участке
            struct MinMax local = GetMinMax(array, begin, end);

            if (by_files) {
                // Пишем результат в файл (добавляем строку "<min> <max>\n")
                FILE *f = fopen(filename, "a");
                if (!f) {
                    fprintf(stderr, "Child cannot open file\n");
                    exit(1);
                }
                fprintf(f, "%d %d\n", local.min, local.max);
                fclose(f);
            } else {
                // Пишем структуру в pipe
                close(pipefd[i][0]); // закрываем конец для чтения
                write(pipefd[i][1], &local, sizeof(struct MinMax));
                close(pipefd[i][1]);
            }

            // Завершение дочернего процесса
            exit(0);
        } else {
            // --- Родительский процесс ---
            child_pids[i] = pid; // запоминаем PID ребёнка
        }
    }

    // --- Родительский процесс: настройка таймаута, если он задан ---

    if (timeout > 0) {
        // Устанавливаем обработчик сигнала SIGALRM
        signal(SIGALRM, alarm_handler);
        // Запускаем таймер на timeout секунд
        alarm(timeout);
    }

    int children_left = pnum;
    int status;

    // Цикл ожидания завершения детей с неблокирующим waitpid и проверкой таймаута
    while (children_left > 0) {
        pid_t res = waitpid(-1, &status, WNOHANG); // -1 = любой ребёнок

        if (res > 0) {
            // Один ребёнок завершился
            children_left--;
        } else if (res == 0) {
            // Никто пока не завершился

            if (timeout_expired) {
                // Время истекло: убиваем всех ещё живых детей
                for (int i = 0; i < pnum; i++) {
                    if (child_pids[i] > 0) {
                        kill(child_pids[i], SIGKILL);
                    }
                }
                // После этого продолжим вызывать waitpid, чтобы "подобрать" всех
            }

            // Немного подождём, чтобы не крутить пустой цикл
            usleep(1000); // 1 мс
        } else {
            // res == -1, скорее всего детей больше нет
            break;
        }
    }

    // --- Сбор результатов ---

    if (by_files) {
        FILE *f = fopen(filename, "r");
        if (!f) {
            fprintf(stderr, "Parent cannot open file for reading\n");
            free(array);
            free(child_pids);
            if (!by_files) free(pipefd);
            return 1;
        }
        int mn, mx;
        while (fscanf(f, "%d %d", &mn, &mx) == 2) {
            if (mn < global_min_max.min) global_min_max.min = mn;
            if (mx > global_min_max.max) global_min_max.max = mx;
        }
        fclose(f);
    } else {
        for (int i = 0; i < pnum; i++) {
            close(pipefd[i][1]); // закрываем конец записи (на всякий случай)
            struct MinMax local;
            if (read(pipefd[i][0], &local, sizeof(struct MinMax)) == sizeof(struct MinMax)) {
                if (local.min < global_min_max.min) global_min_max.min = local.min;
                if (local.max > global_min_max.max) global_min_max.max = local.max;
            }
            close(pipefd[i][0]);
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    printf("Min: %d\n", global_min_max.min);
    printf("Max: %d\n", global_min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);

    free(array);
    free(child_pids);
    if (!by_files) free(pipefd);

    return 0;
}
