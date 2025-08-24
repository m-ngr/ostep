#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sched.h>
#include<time.h>

#define DEF_LOOPS 1000000

typedef unsigned long long ull;

void pin_to_cpu(int cpu) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);

  if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
    fprintf(stderr, "PIN_CPU_ERROR: %s\n", strerror(errno));
    exit(errno);
  }
}

void Pipe(int* pipefd){
  if (pipe(pipefd) == 0) return;
  fprintf(stderr, "PIPE_ERROR: %s\n", strerror(errno));
  exit(errno);
}

int Fork(){
  int rc = fork();
  if (rc >= 0) return rc;
  fprintf(stderr, "FORK_ERROR: %s\n", strerror(errno));
  exit(errno);
}

ull time_diff(struct timespec *start, struct timespec *end) {
  time_t sec = end->tv_sec - start->tv_sec;
  long nsec = end->tv_nsec - start->tv_nsec;
  return (ull)sec * 1000000000ULL + nsec;
}

void parent_code(int pfd1[2], int pfd2[2], ull loops){
  char buf[1];
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  for(ull i = 0; i < loops; ++i){
    write(pfd1[1],"A", 1);
    read(pfd2[0], buf, 1);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  
  ull diff = time_diff(&start, &end);
  printf("AVG Context Switch Time = %lf nsec\n",(double)diff/(2*loops));
}

void child_code(int pfd1[2], int pfd2[2], ull loops){
  char buf[1];
  for(ull i = 0; i < loops; ++i){
    read(pfd1[0], buf, 1);
    write(pfd2[1],"B", 1);
  }
}

int main(int argc, char** argv){
  ull loops = DEF_LOOPS;

  if (argc > 1){
    loops = atoll(argv[1]);
    if (loops < 1) loops = DEF_LOOPS;
  }

  int pfd1[2]; 
  int pfd2[2];
  Pipe(pfd1);
  Pipe(pfd2);

  if (Fork() == 0){
    pin_to_cpu(0);
    child_code(pfd1, pfd2, loops);
  }else{
    pin_to_cpu(0);
    parent_code(pfd1, pfd2, loops);
  }

  return 0;
}