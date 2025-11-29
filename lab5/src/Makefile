CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread

.PHONY: all clean

all: mutex_nomutex mutex_mutex factorial deadlock

mutex_nomutex: mutex.c
	$(CC) $(CFLAGS) -o $@ $<

mutex_mutex: mutex.c
	$(CC) $(CFLAGS) -DUSE_MUTEX -o $@ $<

factorial: factorial.c
	$(CC) $(CFLAGS) -o $@ $<

deadlock: deadlock.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f mutex_nomutex mutex_mutex factorial deadlock
