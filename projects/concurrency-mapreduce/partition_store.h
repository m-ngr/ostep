#ifndef __partition_store_h__
#define __partition_store_h__

// Initialize the partitions and allocate memory
void store_init(int num_partitions);

// Clean up all memory allocated in the partitions
void store_free(int num_partitions);

// Insert a key/value pair into a specific partition (used by MR_Emit)
void store_insert(char* key, char* value, int partition_number);

// Sort a specific partition (called before a reducer starts processing it)
void store_sort(int partition_number);

// Get the number of unique/contiguous keys left in a partition
int store_get_size(int partition_number);

// Helper to get the key at a specific index in a sorted partition
char* store_get_key_at(int partition_number, int index);

// The actual iterator function passed to the user's Reduce function
char* store_get_next(char* key, int partition_number);

#endif  // __partition_store_h__