#include "mapreduce.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "partition_store.h"

// The default hash partition function (Provided in the project description)
unsigned long MR_DefaultHashPartition(char* key, int num_partitions) {
  unsigned long hash = 5381;
  int c;
  while ((c = *key++) != '\0') {
    hash = hash * 33 + c;
  }
  return hash % num_partitions;
}

// Track partition configurations globally within mapreduce.c
static int global_num_partitions = 0;
static Partitioner global_partitioner = NULL;

void MR_Emit(char* key, char* value) {
  // 1. Determine which partition this key belongs to
  unsigned long p_num = global_partitioner(key, global_num_partitions);

  // 2. Blindly push it to our data store layer
  store_insert(key, value, p_num);
}

// MapReduce maps this signature directly to the data store's iterator
char* MR_GetNext(char* key, int partition_number) {
  return store_get_next(key, partition_number);
}

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
char** files = NULL;
int files_num = 0;

void files_init(int argc, char* argv[]) {
  files = ++argv;
  files_num = --argc;
}

char* get_next_file() {
  char* res;
  pthread_mutex_lock(&file_lock);
  if (files_num > 0) {
    res = *files;
    ++files;
    --files_num;
  } else {
    res = NULL;
  }
  pthread_mutex_unlock(&file_lock);
  return res;
}

void* mapper_worker(void* args) {
  Mapper map = (Mapper)args;
  char* file;
  while ((file = get_next_file()) != NULL) {
    map(file);
  }
  return NULL;
}

typedef struct ReducerArgs {
  Reducer reduce;
  int partition_number;
} ReducerArgs;

void* reducer_worker(void* args) {
  ReducerArgs* r = (ReducerArgs*)args;
  int i = 0;
  int p_num = r->partition_number;
  while (i < store_get_size(p_num)) {
    char* key = store_get_key_at(p_num, i);
    // The user's reduce function will internally call MR_GetNext
    // which automatically increments store's internal index tracking!
    r->reduce(key, MR_GetNext, p_num);

    // Fast-forward our loop index past all identical keys we just reduced
    while (i < store_get_size(p_num) &&
           strcmp(store_get_key_at(p_num, i), key) == 0) {
      i++;
    }
  }
  return NULL;
}

void MR_Run(int argc, char* argv[], Mapper map, int num_mappers, Reducer reduce,
            int num_reducers, Partitioner partition) {
  // Phase 1: init structures
  global_num_partitions = num_reducers;
  global_partitioner = partition;

  files_init(argc, argv);
  store_init(global_num_partitions);

  pthread_t* mappers = malloc(num_mappers * sizeof(pthread_t));
  pthread_t* reducers = malloc(num_reducers * sizeof(pthread_t));
  ReducerArgs* reducer_args = malloc(num_reducers * sizeof(ReducerArgs));

  // Phase 2: run mappers
  for (int i = 0; i < num_mappers; ++i) {
    pthread_create(&mappers[i], NULL, mapper_worker, (void*)map);
  }

  for (int i = 0; i < num_mappers; ++i) {
    pthread_join(mappers[i], NULL);
  }

  // Phase 3: sort & run reducers
  for (int i = 0; i < num_reducers; ++i) {
    store_sort(i);
    reducer_args[i].partition_number = i;
    reducer_args[i].reduce = reduce;
    pthread_create(&reducers[i], NULL, reducer_worker, (void*)&reducer_args[i]);
  }

  for (int i = 0; i < num_reducers; ++i) {
    pthread_join(reducers[i], NULL);
  }

  // Phase 4: cleanup
  free(mappers);
  free(reducers);
  free(reducer_args);
  store_free(num_reducers);
}
