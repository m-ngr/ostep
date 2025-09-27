#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[]) {
  int n = 1;
  int pid = getpid();

  if (argc > 1) n = atoi(argv[1]);

  if (settickets(n) != 0){
    printf(1, "ERROR: settickets failed\n");
    exit();
  }

  printf(1, "[%d] started with %d tickets\n", pid, n);

  while(1) {
    for (volatile int i = 0; i < 1000; i++) {}

    sleep(0);
  }

  exit();
}
