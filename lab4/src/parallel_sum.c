#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *args) {
  int sum = 0;
  // Проходим по подмассиву от begin (включительно) до end (не включительно)
  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  // Возвращаем сумму как (void*) через приведение к size_t
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
  /*
   *  TODO:
   *  threads_num by command line arguments
   *  array_size by command line arguments
   *	seed by command line arguments
   */

  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  // Простейший парсер аргументов в фиксированном порядке:
  // ./psum --threads_num N --seed N --array_size N
  if (argc != 7) {
    printf("Usage: %s --threads_num num --seed num --array_size num\n", argv[0]);
    return 1;
  }

  // argv[1] == "--threads_num", argv[2] == число
  threads_num = (uint32_t)atoi(argv[2]);
  // argv[3] == "--seed", argv[4] == число
  seed = (uint32_t)atoi(argv[4]);
  // argv[5] == "--array_size", argv[6] == число
  array_size = (uint32_t)atoi(argv[6]);

  if (threads_num == 0 || array_size == 0) {
    printf("threads_num and array_size must be > 0\n");
    return 1;
  }

  /*
   * TODO:
   * your code here
   * Generate array here
   */

  // Выделяем память под массив
  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    printf("Cannot allocate memory for array\n");
    return 1;
  }

  // Объявляем GenerateArray (она реализована в utils.c из ЛР3)
  extern void GenerateArray(int *array, unsigned int array_size, unsigned int seed);
  // Генерация массива (не входит в измерение времени пока)
  GenerateArray(array, array_size, seed);

  // ТЕПЕРЬ, когда threads_num известен, создаём массив потоков и аргументов
  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);
  if (threads == NULL || args == NULL) {
    printf("Cannot allocate memory for threads/args\n");
    free(array);
    return 1;
  }

  // Делим массив на участки между потоками
  int base_chunk = array_size / threads_num;
  int remainder = array_size % threads_num;
  int current = 0;

  for (uint32_t i = 0; i < threads_num; i++) {
    int extra = (i < (uint32_t)remainder) ? 1 : 0;
    args[i].array = array;
    args[i].begin = current;
    args[i].end = current + base_chunk + extra;
    current = args[i].end;
  }

  // Создаём потоки
  for (uint32_t i = 0; i < threads_num; i++) {
    // ВАЖНО: передаём &args[i], а не &args
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      free(array);
      free(threads);
      free(args);
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    // sum сюда вернётся из ThreadSum через return (void*)(size_t)...
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  free(array);
  free(threads);
  free(args);

  printf("Total: %d\n", total_sum);
  return 0;
}
