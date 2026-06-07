#include "partition_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =========================== Value Vector ==============================

/**
 * Allocates and initializes a ValueVector container.
 * Uses calloc to guarantee zeroed metadata and an empty backing pointer array.
 */
ValueVector* ValueVector_make() {
  ValueVector* vec = (ValueVector*)calloc(1, sizeof(ValueVector));
  if (!vec) return NULL;

  vec->capacity = 5;
  vec->size = 0;
  vec->read_index = 0;

  vec->arr = (char**)calloc(vec->capacity, sizeof(char*));
  if (!vec->arr) {
    free(vec);
    return NULL;
  }

  return vec;
}

/**
 * Frees every allocated string in the vector, the backing array, and the vector
 * struct.
 */
void ValueVector_destroy(ValueVector* vec) {
  if (!vec) return;

  if (vec->arr) {
    for (int i = 0; i < vec->size; i++) {
      if (vec->arr[i]) free(vec->arr[i]);
    }
    free(vec->arr);
  }
  free(vec);
}

/**
 * Inserts a duplicate copy of the string into the vector.
 * Handles dynamic resizing via exponential growth when capacity is reached.
 */
void ValueVector_insert(ValueVector* vec, char* value) {
  if (!vec || !value) return;

  // Lazy initialization fallback if array is missing
  if (vec->arr == NULL) {
    vec->capacity = 5;
    vec->size = 0;
    vec->arr = (char**)calloc(vec->capacity, sizeof(char*));
    if (!vec->arr) return;
  }

  // Resize array using growth-factor of 2 when full
  if (vec->size >= vec->capacity) {
    int old_capacity = vec->capacity;
    vec->capacity *= 2;

    char** new_arr = (char**)realloc(vec->arr, vec->capacity * sizeof(char*));
    if (!new_arr) return;
    vec->arr = new_arr;

    // Explicitly clear the newly appended capacity slots to prevent wild
    // pointer errors
    for (int i = old_capacity; i < vec->capacity; i++) {
      vec->arr[i] = NULL;
    }
  }

  // Duplicate string payload and store it securely
  vec->arr[vec->size] = strdup(value);
  vec->size++;
}

/**
 * Moves through elements sequentially. Returns the string at the current
 * index and steps forward. Returns NULL when bounds are exceeded.
 */
char* ValueVector_get_next(ValueVector* vec) {
  if (!vec || !vec->arr || vec->read_index >= vec->size) {
    return NULL;
  }

  char* value = vec->arr[vec->read_index];
  vec->read_index++;
  return value;
}

/**
 * Resets the internal reader pointer back to index 0.
 */
void ValueVector_reset_reader(ValueVector* vec) {
  if (!vec) return;
  vec->read_index = 0;
}

// ============================= Partition ================================

/**
 * Allocates and initializes a Partition instance.
 */
Partition* Partition_make() {
  Partition* part = (Partition*)calloc(1, sizeof(Partition));
  if (!part) return NULL;

  part->capacity = 5;
  part->size = 0;
  part->read_index = 0;
  part->entries = (KeyEntry*)calloc(part->capacity, sizeof(KeyEntry));
  if (!part->entries) {
    free(part);
    return NULL;
  }

  if (pthread_mutex_init(&part->lock, NULL) != 0) {
    free(part->entries);
    free(part);
    return NULL;
  }

  return part;
}

/**
 * Frees all keys and value vectors inside the partition entries.
 */
void Partition_destroy(Partition* part) {
  if (!part) return;

  if (part->entries) {
    for (int i = 0; i < part->size; i++) {
      if (part->entries[i].key) free(part->entries[i].key);
      if (part->entries[i].values) ValueVector_destroy(part->entries[i].values);
    }

    free(part->entries);
  }

  pthread_mutex_destroy(&part->lock);
}

// --- Partition Operations ---

/**
 * Thread-safe insertion. Finds if key already exists.
 * If yes, appends value. If no, grows unique key space and initializes new
 * ValueVector.
 */
void Partition_insert(Partition* part, char* key, char* value) {
  if (!part || !key || !value) return;

  pthread_mutex_lock(&part->lock);

  // 1. Check if unique key already exists
  for (int i = 0; i < part->size; i++) {
    if (strcmp(part->entries[i].key, key) == 0) {
      ValueVector_insert(part->entries[i].values, value);
      pthread_mutex_unlock(&part->lock);
      return;
    }
  }

  // 2. Handle Partition resizing if full
  if (part->size >= part->capacity) {
    int old_capacity = part->capacity;
    part->capacity *= 2;
    KeyEntry* new_entries =
        (KeyEntry*)realloc(part->entries, part->capacity * sizeof(KeyEntry));
    if (!new_entries) {
      pthread_mutex_unlock(&part->lock);
      return;
    }
    part->entries = new_entries;
    memset(&part->entries[old_capacity], 0,
           (part->capacity - old_capacity) * sizeof(KeyEntry));
  }

  // 3. Populate new KeyEntry and allocate the ValueVector heap instance
  int idx = part->size;
  part->entries[idx].key = strdup(key);
  part->entries[idx].values = ValueVector_make();

  // Insert payload value
  ValueVector_insert(part->entries[idx].values, value);
  part->size++;

  pthread_mutex_unlock(&part->lock);
}
/**
 * Comparator helper function for qsort.
 */
static int compare_key_entries(const void* a, const void* b) {
  return strcmp(((KeyEntry*)a)->key, ((KeyEntry*)b)->key);
}

/**
 * Sorts the unique key array alphabetically.
 */
void Partition_sort(Partition* part) {
  if (!part || part->size <= 1) return;
  qsort(part->entries, part->size, sizeof(KeyEntry), compare_key_entries);
}

/**
 * Yields the next unique key string. Moves partition read_index forward.
 */
char* Partition_next_key(Partition* part) {
  if (!part || part->read_index >= part->size) return NULL;

  char* key = part->entries[part->read_index].key;
  part->read_index++;
  return key;
}

/**
 * Returns the next value string for a specific matched key using ValueVector's
 * sequential reader.
 */
char* Partition_next_value(Partition* part, char* key) {
  if (!part || !key) return NULL;

  int last = part->read_index - 1;
  if (0 <= last && last < part->size) {
    if (strcmp(part->entries[last].key, key) == 0) {
      return ValueVector_get_next(part->entries[last].values);
    }
  }

  for (int i = 0; i < part->size; i++) {
    if (strcmp(part->entries[i].key, key) == 0) {
      return ValueVector_get_next(part->entries[i].values);
    }
  }
  return NULL;
}

// ============================ Container ================================

Container* Container_make(int size, Partitioner partitioner) {
  if (size <= 0 || !partitioner) return NULL;
  Container* cont = calloc(1, sizeof(Container));
  if (!cont) return NULL;

  cont->size = size;
  cont->partitioner = partitioner;
  cont->partitions = calloc(size, sizeof(Partition));
  if (cont->partitions == NULL) {
    free(cont);
    return NULL;
  }

  // Explicitly initialize each individual partition allocated in the array
  // block
  for (int i = 0; i < size; ++i) {
    cont->partitions[i].capacity = 5;
    cont->partitions[i].size = 0;
    cont->partitions[i].read_index = 0;
    cont->partitions[i].entries =
        (KeyEntry*)calloc(cont->partitions[i].capacity, sizeof(KeyEntry));
    pthread_mutex_init(&cont->partitions[i].lock, NULL);
  }

  return cont;
}

void Container_destroy(Container* cont) {
  if (!cont) return;

  for (int i = 0; i < cont->size; ++i) {
    Partition_destroy(&cont->partitions[i]);
  }

  free(cont->partitions);
  free(cont);
}

void Container_insert(Container* cont, char* key, char* value) {
  if (!cont || !key || !value) return;
  int idx = cont->partitioner(key, cont->size);
  Partition_insert(&cont->partitions[idx], key, value);
}

void Container_sort(Container* cont) {
  for (int i = 0; i < cont->size; ++i) {
    Partition_sort(&cont->partitions[i]);
  }
}

char* Container_next_key(Container* cont, int partition) {
  if (partition < 0 || partition >= cont->size) return NULL;
  return Partition_next_key(&cont->partitions[partition]);
}

char* Container_next_value(Container* cont, char* key, int partition) {
  if (!key || partition < 0 || partition >= cont->size) return NULL;
  return Partition_next_value(&cont->partitions[partition], key);
}