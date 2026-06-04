#ifndef MMAP_HELPER_H
#define MMAP_HELPER_H

typedef struct MemRegion {
  char* ptr;
  int size;
  int fd;
} MemRegion;

MemRegion mapFile(const char* file);
void unmapFile(MemRegion* m);

#endif