#include "types.h"
#include "defs.h"

static unsigned int rand_seed = 1;

void srand(unsigned int seed) {
  rand_seed = seed;
}

// linear congruential generator
int rand(void) {
  rand_seed = rand_seed * 1664525 + 1013904223;
  return (rand_seed & 0x7fffffff); // return non-negative int
}

// random number in [a, b]
int rand_range(int a, int b) {
  if (a > b) {
    int tmp = a;
    a = b;
    b = tmp;
  }
  
  int span = b - a + 1;
  return a + (rand() % span);
}