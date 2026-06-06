CC = gcc
CFLAGS = -Wall -Werror -O -pthread

all: wordcount

wordcount: main.c mapreduce.c mapreduce.h partition_store.h partition_store.c
	$(CC) $(CFLAGS) main.c partition_store.c mapreduce.c -o wordcount

clean:
	rm -f wordcount output.txt expected.txt