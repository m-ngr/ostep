#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mmap_helper.h"
#include "processor.h"
#include "work_queue.h"

UnitResult* container = NULL;
UnitQueue shared_queue;

void* consumer(void* arg) {
  Unit unit;
  while (queuePop(&shared_queue, &unit)) {
    container[unit.id] = compressUnit(unit.region);
  }
  return NULL;
}

void producer(MemRegion* files, int len) {
  int id_counter = 0;

  for (int i = 0; i < len; ++i) {
    if (files[i].ptr == NULL) continue;
    int count = unitsCount(files[i]);

    for (int k = 0; k < count; ++k) {
      Unit item;
      item.region = unitRegion(files[i], k);
      item.id = id_counter++;
      queuePush(&shared_queue, item);
    }
  }

  queueSetDone(&shared_queue);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("pzip: file1 [file2 ...]\n");
    exit(1);
  }

  // get CPU count
  long numWorkers = sysconf(_SC_NPROCESSORS_ONLN);
  if (numWorkers < 1) numWorkers = 4;

  // map files to memory
  int numFiles = argc - 1;
  MemRegion* files = malloc(numFiles * sizeof(MemRegion));
  int totalUnits = 0;

  for (int i = 0; i < numFiles; ++i) {
    files[i] = mapFile(argv[i + 1]);
    totalUnits += unitsCount(files[i]);
  }

  if (totalUnits == 0) {
    free(files);
    return 0;
  }

  // init structures
  container = calloc(totalUnits, sizeof(UnitResult));
  queueInit(&shared_queue);

  // run consumer threads
  pthread_t* consumers = malloc(numWorkers * sizeof(pthread_t));
  for (int i = 0; i < numWorkers; ++i) {
    pthread_create(&consumers[i], NULL, consumer, NULL);
  }

  // run main as producer
  producer(files, numFiles);

  // join consumer threads
  for (int i = 0; i < numWorkers; ++i) {
    pthread_join(consumers[i], NULL);
  }

  // merge container
  mergeUnitResults(container, totalUnits);

  // cleanup
  queueDestroy(&shared_queue);
  free(consumers);
  free(container);
  for (int i = 0; i < numFiles; ++i) {
    unmapFile(&files[i]);
  }
  free(files);

  return 0;
}