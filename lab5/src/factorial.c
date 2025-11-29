#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct FactorialArgs {
  int begin;
  int end;
  int mod;
  long long *result;
  pthread_mutex_t *mutex;
};

static int parse_int(const char *str) {
  char *end = NULL;
  long value = strtol(str, &end, 10);
  if (errno != 0 || end == str || *end != '\0' || value <= 0) {
    fprintf(stderr, "Invalid numeric argument: %s\n", str);
    exit(EXIT_FAILURE);
  }
  return (int)value;
}

static void *factorial_worker(void *args) {
  struct FactorialArgs *data = (struct FactorialArgs *)args;

  long long local = 1;
  for (int i = data->begin; i <= data->end; i++) {
    local = (local * i) % data->mod;
  }

  if (pthread_mutex_lock(data->mutex) != 0) {
    perror("pthread_mutex_lock");
    pthread_exit(NULL);
  }

  *data->result = (*data->result * local) % data->mod;

  if (pthread_mutex_unlock(data->mutex) != 0) {
    perror("pthread_mutex_unlock");
    pthread_exit(NULL);
  }

  return NULL;
}

int main(int argc, char **argv) {
  int k = -1;
  int pnum = -1;
  int mod = -1;

  const struct option long_options[] = {
      {"k", required_argument, NULL, 'k'},
      {"pnum", required_argument, NULL, 'p'},
      {"mod", required_argument, NULL, 'm'},
      {0, 0, 0, 0}};

  while (true) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "k:p:m:", long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 'k':
      k = parse_int(optarg);
      break;
    case 'p':
      pnum = parse_int(optarg);
      break;
    case 'm':
      mod = parse_int(optarg);
      break;
    default:
      fprintf(stderr,
              "Usage: %s -k <num> -p <threads> -m <mod> (aliases: --k, --pnum, --mod)\n",
              argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (k <= 0 || pnum <= 0 || mod <= 0) {
    fprintf(stderr, "All arguments k, pnum and mod must be positive integers.\n");
    return EXIT_FAILURE;
  }

  pthread_t *threads = calloc(pnum, sizeof(pthread_t));
  struct FactorialArgs *args = calloc(pnum, sizeof(struct FactorialArgs));
  if (threads == NULL || args == NULL) {
    perror("calloc");
    return EXIT_FAILURE;
  }

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  long long result = 1;

  int chunk = k / pnum;
  int remainder = k % pnum;
  int current = 1;

  for (int i = 0; i < pnum; i++) {
    int start = current;
    int finish = current + chunk - 1;
    if (remainder > 0) {
      finish++;
      remainder--;
    }
    if (start > k) {
      start = k;
    }
    if (finish > k) {
      finish = k;
    }

    args[i].begin = start;
    args[i].end = finish;
    args[i].mod = mod;
    args[i].result = &result;
    args[i].mutex = &mutex;

    if (pthread_create(&threads[i], NULL, factorial_worker, &args[i]) != 0) {
      perror("pthread_create");
      free(threads);
      free(args);
      return EXIT_FAILURE;
    }

    current = finish + 1;
  }

  for (int i = 0; i < pnum; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      perror("pthread_join");
      free(threads);
      free(args);
      return EXIT_FAILURE;
    }
  }

  printf("%d! mod %d = %lld\n", k, mod, result % mod);

  free(threads);
  free(args);
  return EXIT_SUCCESS;
}
