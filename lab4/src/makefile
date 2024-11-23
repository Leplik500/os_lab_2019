CC=gcc
CFLAGS=-I.

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

zombie_parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o zombie_parallel_min_max utils.o find_min_max.o zombie_parallel_min_max.c $(CFLAGS)

process_memory: process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

parallel_sum: parallel_sum.c utils.o utils.h
	$(CC)  -o parallel_sum utils.o utils.h parallel_sum.c $(CFLAGS) -lpthread

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o parallel_min_max zombie_parallel_min_max process_memory parallel_sum

all : parallel_min_max zombie_parallel_min_max process_memory parallel_sum