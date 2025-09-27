#ifndef POLICY_H
#define POLICY_H

enum sched_policy { RR = 0, LOTTERY = 1 };

static inline int valid_policy(int p) { return (p == RR || p == LOTTERY); }

static inline const char* policy_str(enum sched_policy p) {
  switch (p) {
    case RR:
      return "Round Robin";
    case LOTTERY:
      return "Lottery";
    default:
      return "UNKNOWN";
  }
}

#endif