#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_mutex_t first = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t second = PTHREAD_MUTEX_INITIALIZER;

static void *lock_first_then_second(void *unused) {
  (void)unused;
  printf("Thread A locking first mutex...\n");
  pthread_mutex_lock(&first);
  sleep(1);
  printf("Thread A locking second mutex... (will wait)\n");
  pthread_mutex_lock(&second);
  printf("Thread A acquired both locks (unexpected)\n");
  pthread_mutex_unlock(&second);
  pthread_mutex_unlock(&first);
  return NULL;
}

static void *lock_second_then_first(void *unused) {
  (void)unused;
  printf("Thread B locking second mutex...\n");
  pthread_mutex_lock(&second);
  sleep(1);
  printf("Thread B locking first mutex... (will wait)\n");
  pthread_mutex_lock(&first);
  printf("Thread B acquired both locks (unexpected)\n");
  pthread_mutex_unlock(&first);
  pthread_mutex_unlock(&second);
  return NULL;
}

int main(void) {
  pthread_t a, b;

  if (pthread_create(&a, NULL, lock_first_then_second, NULL) != 0) {
    perror("pthread_create");
    return EXIT_FAILURE;
  }

  if (pthread_create(&b, NULL, lock_second_then_first, NULL) != 0) {
    perror("pthread_create");
    return EXIT_FAILURE;
  }

  printf("Both threads started. Expect deadlock after initial locks...\n");

  sleep(3);
  printf("If you see this message and the program does not exit, threads are deadlocked.\n");
  printf("Press Ctrl+C to terminate or wait for the process to be killed by a timeout.\n");

  pthread_cancel(a);
  pthread_cancel(b);
  pthread_mutex_unlock(&first);
  pthread_mutex_unlock(&second);

  return EXIT_SUCCESS;
}
