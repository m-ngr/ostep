#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapreduce.h"

// The User-defined Map function
void Map(char* file_name) {
  FILE* fp = fopen(file_name, "r");
  if (fp == NULL) {
    perror("Fopen failed");
    exit(1);
  }

  char* line = NULL;
  size_t size = 0;
  while (getline(&line, &size, fp) != -1) {
    char *token, *dummy = line;
    // Split line by spaces, tabs, and newlines
    while ((token = strsep(&dummy, " \t\n\r")) != NULL) {
      if (strlen(token) > 0) {
        MR_Emit(token, "1");
      }
    }
  }
  free(line);
  fclose(fp);
}

// The User-defined Reduce function
void Reduce(char* key, Getter get_next, int partition_number) {
  int count = 0;
  char* value;
  // Keep pulling values for this specific key until NULL
  while ((value = get_next(key, partition_number)) != NULL) {
    count++;
  }
  // Print the result to stdout (tracked by our test script)
  printf("%s %d\n", key, count);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <input_file1> [input_file2 ...]\n", argv[0]);
    exit(1);
  }

  // Run MapReduce with 4 Mapper threads and 4 Reducer threads
  MR_Run(argc, argv, Map, 2, Reduce, 4, MR_DefaultHashPartition);

  return 0;
}