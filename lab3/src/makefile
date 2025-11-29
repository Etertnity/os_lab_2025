# Задание 3: Makefile со сборкой всех программ
# CC — компилятор, CFLAGS — флаги компиляции
CC=gcc
CFLAGS=-I.

# Target all — собирает все задачи лабораторной
all: sequential_min_max parallel_min_max run_sequential_min_max

# Сборка последовательной версии
sequential_min_max: utils.o find_min_max.o
	$(CC) -o sequential_min_max sequential_min_max.c utils.o find_min_max.o $(CFLAGS)

# Сборка параллельной версии
parallel_min_max: utils.o find_min_max.o
	$(CC) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o $(CFLAGS)

# Сборка программы с exec
run_sequential_min_max: run_sequential_min_max.c
	$(CC) -o run_sequential_min_max run_sequential_min_max.c $(CFLAGS)

# Компиляция объектных файлов
utils.o: utils.c utils.h
	$(CC) -c utils.c -o utils.o

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) -c find_min_max.c -o find_min_max.o

# Очистка проекта
clean:
	rm -f *.o sequential_min_max parallel_min_max run_sequential_min_max
