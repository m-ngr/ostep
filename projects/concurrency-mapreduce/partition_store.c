#include "partition_store.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char* key;
  char* value;
} KVNode;

typedef struct {
  KVNode* pairs;
  int capacity;
  int size;
  int current_idx;       // Tracks the iterator position for get_next
  pthread_mutex_t lock;  // Protects this partition during mapper inserts
} PartitionStore;

// Global array of partitions, sized to num_partitions (num_reducers)
static PartitionStore* partitions = NULL;

void store_init(int num_partitions) {
  partitions = malloc(num_partitions * sizeof(PartitionStore));
  for (int i = 0; i < num_partitions; i++) {
    partitions[i].pairs = malloc(32 * sizeof(KVNode));  // start small
    partitions[i].capacity = 32;
    partitions[i].size = 0;
    partitions[i].current_idx = 0;
    pthread_mutex_init(&partitions[i].lock, NULL);
  }
}

void store_insert(char* key, char* value, int partition_number) {
  PartitionStore* p = &partitions[partition_number];

  pthread_mutex_lock(&p->lock);

  // Dynamic array resizing (doubling capacity)
  if (p->size >= p->capacity) {
    p->capacity *= 2;
    p->pairs = realloc(p->pairs, p->capacity * sizeof(KVNode));
    if (p->pairs == NULL) {
      perror("Realloc failed in store_insert");
      exit(1);
    }
  }

  // OSTEP mandate: The library must make copies of keys and values
  p->pairs[p->size].key = strdup(key);
  p->pairs[p->size].value = strdup(value);
  p->size++;

  pthread_mutex_unlock(&p->lock);
}

// Comparator function for qsort
static int compare_keys(const void* a, const void* b) {
  return strcmp(((KVNode*)a)->key, ((KVNode*)b)->key);
}

void store_sort(int partition_number) {
  PartitionStore* p = &partitions[partition_number];
  if (p->size > 0) {
    qsort(p->pairs, p->size, sizeof(KVNode), compare_keys);
  }
}

int store_get_size(int partition_number) {
  return partitions[partition_number].size;
}

char* store_get_key_at(int partition_number, int index) {
  return partitions[partition_number].pairs[index].key;
}

char* store_get_next(char* key, int partition_number) {
  PartitionStore* p = &partitions[partition_number];

  // If we've processed everything in this partition, return NULL
  if (p->current_idx >= p->size) {
    return NULL;
  }

  // Check if the key at our tracking index matches the requested key
  if (strcmp(p->pairs[p->current_idx].key, key) == 0) {
    char* val = p->pairs[p->current_idx].value;
    p->current_idx++;  // Advance the index for the next call
    return val;
  }

  // Key changed or does not match
  return NULL;
}

void store_free(int num_partitions) {
  if (!partitions) return;
  for (int i = 0; i < num_partitions; i++) {
    for (int j = 0; j < partitions[i].size; j++) {
      free(partitions[i].pairs[j].key);
      free(partitions[i].pairs[j].value);
    }
    free(partitions[i].pairs);
    pthread_mutex_destroy(&partitions[i].lock);
  }
  free(partitions);
}