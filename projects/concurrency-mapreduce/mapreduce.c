#include "mapreduce.h"

#include <stdio.h>
#include <stdlib.h>

// Stub for emitting a key/value pair from mappers
void MR_Emit(char* key, char* value) {
  // TODO: Implement internal storage of key/value pairs into partitions
  (void)key;    // Suppress unused parameter warning
  (void)value;  // Suppress unused parameter warning
}

// The default hash partition function (Provided in the project description)
unsigned long MR_DefaultHashPartition(char * key, int num_partitions) {
  unsigned long hash = 5381;
  int c;
  while ((c = *key++) != '\0') {
    hash = hash * 33 + c;
  }
  return hash % num_partitions;
}

// Stub for the primary orchestrator function
void MR_Run(int argc, char* argv[], Mapper map, int num_mappers, Reducer reduce,
            int num_reducers, Partitioner partition) {
  // Suppress unused parameter warnings for compiling with -Werror
  (void)argc;
  (void)argv;
  (void)map;
  (void)num_mappers;
  (void)reduce;
  (void)num_reducers;
  (void)partition;

  printf(
      "MR_Run stub called. Threads and structures are not yet implemented.\n");
}