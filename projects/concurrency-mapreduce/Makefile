CC = gcc
CFLAGS = -Wall -Werror -O -pthread

all: wordcount

wordcount: main.c mapreduce.c mapreduce.h
	$(CC) $(CFLAGS) main.c mapreduce.c -o wordcount

clean:
	rm -f wordcount output.txt expected.txt