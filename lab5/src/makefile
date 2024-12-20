CC = gcc
CFLAGS = -pthread

SOURCES = deadlock.c factorial.c with_mutex.c without_mutex.c
EXECUTABLES = deadlock factorial with_mutex without_mutex

all: $(EXECUTABLES)

deadlock: deadlock.c
	$(CC) $(CFLAGS) -o $@ $<

factorial: factorial.c
	$(CC) $(CFLAGS) -o $@ $<

with_mutex: with_mutex.c
	$(CC) $(CFLAGS) -o $@ $<

without_mutex: without_mutex.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean
