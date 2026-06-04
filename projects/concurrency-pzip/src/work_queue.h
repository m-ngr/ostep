#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include "mmap_helper.h"

#define QUEUE_CAPACITY 16

typedef struct Unit {
  MemRegion region;
  int id;
} Unit;

typedef struct UnitQueue {
  Unit buffer[QUEUE_CAPACITY];
  int head;
  int tail;
  int size;
  int done;
  pthread_mutex_t mutex;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
} UnitQueue;

void queueInit(UnitQueue* q);
void queuePush(UnitQueue* q, Unit unit);
int queuePop(UnitQueue* q, Unit* unit);  // Returns 0 when completely done/exhausted
void queueSetDone(UnitQueue* q);
void queueDestroy(UnitQueue* q);

#endif