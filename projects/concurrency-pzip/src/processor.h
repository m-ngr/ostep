#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>
#include "mmap_helper.h"

#define USIZE 4048 // 4KB Unit Size

typedef struct RLEEntry {
  int32_t count;
  char character;
} RLEEntry;

typedef struct UnitResult {
  RLEEntry* entries;
  int count;
} UnitResult;

UnitResult compressUnit(MemRegion u);
void mergeUnitResults(UnitResult* container, int totalUnits);

int unitsCount(MemRegion m);
MemRegion unitRegion(MemRegion m, int index);

#endif