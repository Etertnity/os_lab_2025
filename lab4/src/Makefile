CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread

# Главная цель — собрать обе программы
all: zombie_demo process_memory psum

# Сборка задания 1 (зомби)
zombie_demo: zombie_demo.c
	$(CC) $(CFLAGS) -o zombie_demo zombie_demo.c

# Сборка задания 3 (виртуальная память)
process_memory: process_memory.c
	$(CC) $(CFLAGS) -o process_memory process_memory.c

psum: parallel_sum.c utils.c
	$(CC) $(CFLAGS) -o psum parallel_sum.c utils.c

# Очистка мусора
clean:
	rm -f zombie_demo process_memory psum
