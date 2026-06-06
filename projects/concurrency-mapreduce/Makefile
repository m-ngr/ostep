CC = gcc
CFLAGS = -Wall -Werror -O -pthread

all: wordcount

wordcount: main.c mapreduce.c mapreduce.h partition_store.c partition_store.h
	$(CC) $(CFLAGS) main.c partition_store.c mapreduce.c -o wordcount

clean:
	@echo "=== Sweeping Workspace Clean ==="
	rm -f wordcount
	rm -f output.txt expected.txt raw_output.txt valgrind_log.txt
	rm -f *_valgrind_failed.txt
	rm -rf \#inputs/