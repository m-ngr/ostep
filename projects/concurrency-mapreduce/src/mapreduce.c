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

static Container* MR_Container = NULL;

void MR_Emit(char* key, char* value) {
  Container_insert(MR_Container, key, value);
}

char* MR_GetNext(char* key, int partition_number) {
  return Container_next_value(MR_Container, key, partition_number);
}

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
char** files = NULL;
int files_num = 0;

void files_init(int argc, char* argv[]) {
  files = ++argv;
  files_num = --argc;
}

char* get_next_file() {
  char* res = NULL;
  pthread_mutex_lock(&file_lock);
  if (files_num > 0) {
    res = *files;
    ++files;
    --files_num;
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
  char* key = NULL;
  int p = r->partition_number;
  while ((key = Container_next_key(MR_Container, p)) != NULL) {
    r->reduce(key, MR_GetNext, p);
  }
  return NULL;
}

void MR_Run(int argc, char* argv[], Mapper map, int num_mappers, Reducer reduce,
            int num_reducers, Partitioner partition) {
  // Phase 1: init structures
  Partitioner pfn = (partition == NULL) ? MR_DefaultHashPartition : partition;
  MR_Container = Container_make(num_reducers, pfn);
  files_init(argc, argv);

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

  // Phase 3: Sort
  Container_sort(MR_Container);

  // Phase 4: run reducers
  for (int i = 0; i < num_reducers; ++i) {
    reducer_args[i].partition_number = i;
    reducer_args[i].reduce = reduce;
    pthread_create(&reducers[i], NULL, reducer_worker, (void*)&reducer_args[i]);
  }

  for (int i = 0; i < num_reducers; ++i) {
    pthread_join(reducers[i], NULL);
  }

  // Phase 5: cleanup
  free(mappers);
  free(reducers);
  free(reducer_args);
  Container_destroy(MR_Container);
}
