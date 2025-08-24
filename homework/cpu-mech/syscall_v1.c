#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>
#include<stdlib.h>

#define N 99999999
#define R 10

typedef unsigned long long ull;

ull time_diff(struct timeval *start, struct timeval *end){
  time_t sec = end->tv_sec - start->tv_sec;
  suseconds_t usec = end->tv_usec - start->tv_usec;
  ull diff = sec * 1000000 + usec;
  return diff;
}

ull run_syscalls(ull calls){
  struct timeval start;
  struct timeval end;
  gettimeofday(&start, NULL);
  for(ull i = 0; i < calls; ++i){
    getpid();
  }
  gettimeofday(&end, NULL);
  ull diff = time_diff(&start, &end);
  return diff;
}

int main(int argc, char** argv){
  int rounds = R;
  ull calls = N;

  if (argc > 1){
    calls = atoll(argv[1]);
    if (calls < 1) calls = N;
  }

  if (argc > 2){
    rounds = atoi(argv[2]);
    if (rounds < 1) rounds = R;
  }

  ull sum = 0;

  for(int r = 0; r < rounds; ++r){
    ull diff = run_syscalls(calls);
    sum += diff;
    printf("[%03d] diff = %llu usec\n", r+1, diff);
  }

  ull avg = sum/rounds;
  printf("------------------------------\n");
  printf("[AVG] diff = %llu usec\n", avg);
  printf("==============================\n");
  printf("AvgSysCall = %lf nsec\n", ((double)avg/calls)*1000);
  return 0;
}