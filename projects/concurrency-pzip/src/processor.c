#include "processor.h"
#include <stdlib.h>
#include <stdio.h>

UnitResult compressUnit(MemRegion u) {
  UnitResult res = {.entries = malloc(u.size * sizeof(RLEEntry)), .count = 0};
  if (u.size == 0) return res;

  char last = u.ptr[0];
  int count = 1;

  for (int i = 1; i < u.size; ++i) {
    char current = u.ptr[i];
    if (current == last) {
      ++count;
    } else {
      res.entries[res.count++] = (RLEEntry){.count = count, .character = last};
      last = current;
      count = 1;
    }
  }
  res.entries[res.count++] = (RLEEntry){.count = count, .character = last};
  return res;
}

void mergeUnitResults(UnitResult* container, int totalUnits) {
  int32_t tracked_count = 0;
  char tracked_char = 0;
  uint8_t out_buffer[5];

  for (int i = 0; i < totalUnits; ++i) {
    UnitResult ur = container[i];
    for (int j = 0; j < ur.count; ++j) {
      RLEEntry current = ur.entries[j];

      if (tracked_count == 0) {
        tracked_char = current.character;
        tracked_count = current.count;
      } else if (current.character == tracked_char) {
        tracked_count += current.count;
      } else {
        *(int32_t*)out_buffer = tracked_count;
        out_buffer[4] = (unsigned char)tracked_char;
        fwrite(out_buffer, 5, 1, stdout);

        tracked_char = current.character;
        tracked_count = current.count;
      }
    }
    free(ur.entries);
  }

  if (tracked_count > 0) {
    *(int32_t*)out_buffer = tracked_count;
    out_buffer[4] = (unsigned char)tracked_char;
    fwrite(out_buffer, 5, 1, stdout);
  }
}

int unitsCount(MemRegion m) {
  if (m.size <= 0 || m.ptr == NULL) return 0;
  int count = m.size / USIZE;
  return m.size % USIZE == 0 ? count : count + 1;
}

MemRegion unitRegion(MemRegion m, int index) {
  MemRegion u = {.ptr = NULL, .size = 0, .fd = -1};
  int count = unitsCount(m);
  
  if (index < 0 || index >= count) return u;
  u.ptr = (char*)m.ptr + (index * USIZE);

  if (index == count - 1 && m.size % USIZE != 0) {
    u.size = m.size % USIZE; 
  } else {
    u.size = USIZE;          
  }

  u.fd = -1; 

  return u;
}
