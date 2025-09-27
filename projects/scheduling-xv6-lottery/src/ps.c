#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "policy.h"

void ps(){
  struct pstat ps;
  int rc = getpinfo(&ps);

  if (rc == -1){
    printf(1, "ERROR: getpinfo failed\n");
    exit();
  }

  printf(1, "CPU STATUS: tickets = %d\t ticks = %d\t policy = %s\n",
    ps.all_tickets,
    ps.all_ticks,
    policy_str(ps.policy)
  );


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



void usage(void) {
  printf(1, "Usage:\n");
  printf(1, "  ps             : show process stats\n");
  printf(1, "  ps r           : reset CPU counters, then show process stats\n");
  printf(1, "  ps set <p>     : set scheduling policy (0=RR, 1=Lottery)\n");
  exit();
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    ps();
    exit();
  }

  if (argc == 2 && strcmp(argv[1], "r") == 0) {
    cpureset();
    ps();
    exit();
  } 

  if (argc == 3 && strcmp(argv[1], "set") == 0) {
    int p = atoi(argv[2]);
    if (!valid_policy(p)) {
      printf(1, "ERROR: invalid policy %d\n", p);
      usage();
    }
    setpolicy(p);
    printf(1, "Scheduling policy set to %s\n", policy_str(p));
    exit();
  }

  usage();
}
