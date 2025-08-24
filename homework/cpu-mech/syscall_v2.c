#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h> 

#define DEF_CALLS 10000000
#define DEF_ROUNDS 10
#define DEF_FUNCTION "read"  

typedef unsigned long long ull;
typedef void (*test_func_t)(); 

// =========================TEST CALLS=========================
void test_getpid() { getpid();}
void test_getppid() {getppid();}
void test_getuid() {getuid();}
void test_read() {
  char buf[1];
  read(STDIN_FILENO, buf, 0);
}
// ============================================================
ull time_diff(struct timespec *start, struct timespec *end) {
  time_t sec = end->tv_sec - start->tv_sec;
  long nsec = end->tv_nsec - start->tv_nsec;
  return (ull)sec * 1000000000ULL + nsec;
}

test_func_t select_func(const char *name) {
  if (strcmp(name, "getpid") == 0) return test_getpid;
  if (strcmp(name, "getppid") == 0) return test_getppid;
  if (strcmp(name, "getuid") == 0) return test_getuid;
  if (strcmp(name, "read") == 0) return test_read;
  return test_read;
}


ull run_syscalls(test_func_t fn, ull calls) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  
  for (ull i = 0; i < calls; ++i) {
    fn();
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  return time_diff(&start, &end);
}

int main(int argc, char **argv) {
  int rounds = DEF_ROUNDS;
  ull calls = DEF_CALLS;
  const char *func_name = DEF_FUNCTION;

  if (argc > 1) func_name = argv[1];

  if (argc > 2) {
    calls = atoll(argv[2]);
    if (calls < 1) calls = DEF_CALLS;
  }

  if (argc > 3) {
    rounds = atoi(argv[3]);
    if (rounds < 1) rounds = DEF_ROUNDS;
  }

  ull sum = 0;
  test_func_t fn = select_func(func_name);

  for (int r = 0; r < rounds; ++r) {
    ull diff = run_syscalls(fn, calls);
    sum += diff;
    printf("[%03d] diff = %llu nsec\n", r+1, diff);
  }

  ull avg = sum / rounds;
  printf("------------------------------\n");
  printf("[AVG] diff = %llu nsec\n", avg);
  printf("==============================\n");
  printf("AvgSysCall = %lf nsec\n", (double)avg / calls);
  return 0;
}
