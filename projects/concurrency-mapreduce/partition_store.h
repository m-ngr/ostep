#ifndef __partition_store_h__
#define __partition_store_h__
#include <pthread.h>

#include "mapreduce.h"

typedef struct {
  char** arr;
  int size;
  int capacity;
  int read_index;
} ValueVector;

typedef struct {
  char* key;
  ValueVector* values;
} KeyEntry;

typedef struct {
  KeyEntry* entries;
  int size;
  int capacity;
  int read_index;
  pthread_mutex_t lock;
} Partition;

typedef struct {
  Partition* partitions;
  int size;
  Partitioner partitioner;
} Container;

Container* Container_make(int size, Partitioner partitioner);
void Container_destroy(Container* cont);
void Container_insert(Container* cont, char* key, char* value);
void Container_sort(Container* cont);
char* Container_next_key(Container* cont, int partition);
char* Container_next_value(Container* cont, char* key, int partition);

#endif  // __partition_store_h__