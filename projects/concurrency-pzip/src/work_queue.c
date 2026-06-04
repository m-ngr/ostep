#include "work_queue.h"

void queueInit(UnitQueue* q) {
  q->head = 0;
  q->tail = 0;
  q->size = 0;
  q->done = 0;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->not_full, NULL);
  pthread_cond_init(&q->not_empty, NULL);
}

void queuePush(UnitQueue* q, Unit item) {
  pthread_mutex_lock(&q->mutex);
  while (q->size == QUEUE_CAPACITY) {
    pthread_cond_wait(&q->not_full, &q->mutex);
  }
  q->buffer[q->tail] = item;
  q->tail = (q->tail + 1) % QUEUE_CAPACITY;
  q->size++;
  pthread_cond_signal(&q->not_empty);
  pthread_mutex_unlock(&q->mutex);
}

int queuePop(UnitQueue* q, Unit* item) {
  pthread_mutex_lock(&q->mutex);
  while (q->size == 0 && !q->done) {
    pthread_cond_wait(&q->not_empty, &q->mutex);
  }
  if (q->size == 0 && q->done) {
    pthread_mutex_unlock(&q->mutex);
    return 0; // Signals execution termination
  }
  *item = q->buffer[q->head];
  q->head = (q->head + 1) % QUEUE_CAPACITY;
  q->size--;
  pthread_cond_signal(&q->not_full);
  pthread_mutex_unlock(&q->mutex);
  return 1;
}

void queueSetDone(UnitQueue* q) {
  pthread_mutex_lock(&q->mutex);
  q->done = 1;
  pthread_cond_broadcast(&q->not_empty);
  pthread_mutex_unlock(&q->mutex);
}

void queueDestroy(UnitQueue* q) {
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->not_full);
  pthread_cond_destroy(&q->not_empty);
}