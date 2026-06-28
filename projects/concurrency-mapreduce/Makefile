CC = gcc
CFLAGS = -Wall -Werror -O -pthread
SRC_DIR = ./src

all: wordcount

wordcount: $(SRC_DIR)/main.c $(SRC_DIR)/partition_store.c $(SRC_DIR)/mapreduce.c $(SRC_DIR)/mapreduce.h $(SRC_DIR)/partition_store.h
	$(CC) $(CFLAGS) $(SRC_DIR)/main.c $(SRC_DIR)/partition_store.c $(SRC_DIR)/mapreduce.c -o wordcount

clean:
	@echo "=== Sweeping Workspace Clean ==="
	rm -f wordcount go/wordcount
	rm -f output.txt expected.txt raw_output.txt valgrind_log.txt helgrind_log.txt go_race_log.txt
	rm -f *_valgrind_failed.txt *_helgrind_failed.txt *_go_race_failed.txt
	rm -rf \#inputs/