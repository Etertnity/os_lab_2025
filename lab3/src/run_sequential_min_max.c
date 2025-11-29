// Задание 4: Запуск программы sequential_min_max через fork + exec
// Здесь exec полностью заменяет процесс дочернего процесса новой программой.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s seed array_size\n", argv[0]);
        return 1;
    }

    // Создаём новый процесс
    pid_t pid = fork();

    if (pid == 0) {
        // exec заменяет текущий процесс программой sequential_min_max
        execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2], NULL);

        perror("execl failed");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    printf("Child exited with code %d\n", WEXITSTATUS(status));
    return 0;
}