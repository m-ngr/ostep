#include "mmap_helper.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

MemRegion mapFile(const char* file) {
  MemRegion res = {.ptr = NULL, .size = 0, .fd = -1};
  int fd = open(file, O_RDONLY);
  if (fd == -1) return res;

  struct stat sb;
  if (fstat(fd, &sb) == -1 || sb.st_size == 0) {
    close(fd);
    return res;
  }

  void* file_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (file_in_memory == MAP_FAILED) {
    close(fd);
    return res;
  }

  res.ptr = file_in_memory;
  res.size = sb.st_size;
  res.fd = fd;
  return res;
}

void unmapFile(MemRegion* m) {
  if (m == NULL || m->ptr == NULL) return;
  if (m->fd != -1) {
    munmap(m->ptr, m->size);
    close(m->fd);
  }
  m->ptr = NULL;
  m->size = 0;
  m->fd = -1;
}