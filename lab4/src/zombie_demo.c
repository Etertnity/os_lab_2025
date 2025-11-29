// Демонстрация зомби-процесса в одном терминале
// 1) запускаем дочерний процесс, который сразу завершится
// 2) родитель НЕ вызывает wait() сразу, а показывает ps с состоянием Z
// 3) потом вызывает wait(), и повторно показывает ps — зомби исчез

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // fork, sleep
#include <sys/types.h> // pid_t
#include <sys/wait.h>  // wait

int main() {
    pid_t pid = fork(); // создаём дочерний процесс

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // ---- Дочерний процесс ----
        printf("[CHILD] PID = %d, PPID = %d. Сейчас завершаюсь.\n",
               getpid(), getppid());
        // ребёнок сразу завершается → переходит в состояние "завершён",
        // но пока родитель не вызовет wait(), он будет числиться как зомби (Z)
        exit(0);
    } else {
        // ---- Родительский процесс ----
        printf("[PARENT] PID = %d. Породил ребёнка PID = %d\n",
               getpid(), pid);

        // Немного подождём, чтобы ребёнок точно успел закончить
        sleep(1);

        printf("\n=== ЭТАП 1: ребёнок уже завершился, но wait() ЕЩЁ НЕ вызван ===\n");
        printf("Ожидаем, что ребёнок сейчас будет в состоянии Z (zombie).\n");
        printf("Вывод ps для конкретного PID:\n\n");

        // Сформируем команду ps для конкретного PID
        char cmd[256];
        snprintf(cmd, sizeof(cmd),
                 "ps -o pid,ppid,stat,cmd -p %d", pid);
        int ret = system(cmd); // вызовем ps через system()
        (void)ret; // чтобы не ругался компилятор

        sleep(5);

        printf("\n=== ЭТАП 2: вызываем wait(), чтобы убрать зомби ===\n");
        int status;
        pid_t waited = wait(&status); // "подбираем" завершившегося ребёнка

        if (waited == -1) {
            perror("wait failed");
        } else {
            printf("[PARENT] wait() вернул PID = %d. Зомби должен исчезнуть.\n",
                   waited);
        }

        printf("\nПроверим ещё раз через ps:\n\n");
        ret = system(cmd);

        printf("[PARENT] Завершаюсь.\n");

        sleep(3);
    }

    return 0;
}
