#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


void ps(){
  struct pstat ps;
  int rc = getpinfo(&ps);

  if (rc == -1){
    printf(1, "ERROR: getpinfo failed\n");
    exit();
  }

  printf(1, "CPU STATUS: tickets = %d\t ticks = %d\n", ps.all_tickets, ps.all_ticks);
  printf(1, "pid\ttickets\tticks\tusage\ttarget\tdiff\n");

  for(int i = 0; i < NPROC; ++i) {
    if (ps.inuse[i] == 0) continue;

    int usage = ps.ticks[i]*100/ps.all_ticks;
    int target = ps.tickets[i]*100/ps.all_tickets;
    int diff = usage - target;

    printf(1, "%d\t%d\t%d\t%d%%\t%d%%\t%d%%\n", 
      ps.pid[i],
      ps.tickets[i],
      ps.ticks[i],
      usage,
      target,
      diff
    );
  }
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "r") == 0) cpureset();
  }

  ps();
  exit();
}
