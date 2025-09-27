#ifndef POLICY_H
#define POLICY_H

enum sched_policy { RR = 0, LOTTERY = 1, MLFQ = 2};

static inline const DEFAULT_POLICY = LOTTERY;

static inline int valid_policy(int p) { return (p == RR || p == LOTTERY || p == MLFQ); }

static inline const char* policy_str(enum sched_policy p) {
  switch (p) {
    case RR:
      return "Round Robin";
    case LOTTERY:
      return "Lottery";
    case MLFQ:
      return "MLFQ";
    default:
      return "UNKNOWN";
  }
}

#endif