# Thread API Homework

Homework [overview](./overview.md)

## Questions

1. First build `main-race.c`. Examine the code so you can see the (hopefully obvious) data race in the code. Now run helgrind (by typing `valgrind --tool=helgrind ./main-race`) to see how it reports the race. Does it point to the right lines of code? What other information does it give to you?

**Answer:** helgrind output shows data race on `balance` at lines **8** and **15**

```txt
==13867== ----------------------------------------------------------------
==13867==
==13867== Possible data race during read of size 4 at 0x10C014 by thread #1
==13867== Locks held: none
==13867==    at 0x109236: main (main-race.c:15)
==13867==
==13867== This conflicts with a previous write of size 4 by thread #2
==13867== Locks held: none
==13867==    at 0x1091BE: worker (main-race.c:8)
==13867==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==13867==    by 0x4910AA3: start_thread (pthread_create.c:447)
==13867==    by 0x499DA63: clone (clone.S:100)
==13867==  Address 0x10c014 is 0 bytes inside data symbol "balance"
==13867==
==13867== ----------------------------------------------------------------
==13867==
==13867== Possible data race during write of size 4 at 0x10C014 by thread #1
==13867== Locks held: none
==13867==    at 0x10923F: main (main-race.c:15)
==13867==
==13867== This conflicts with a previous write of size 4 by thread #2
==13867== Locks held: none
==13867==    at 0x1091BE: worker (main-race.c:8)
==13867==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==13867==    by 0x4910AA3: start_thread (pthread_create.c:447)
==13867==    by 0x499DA63: clone (clone.S:100)
==13867==  Address 0x10c014 is 0 bytes inside data symbol "balance"
```

---

2. What happens when you remove one of the offending lines of code? Now add a lock around one of the updates to the shared variable, and then around both. What does helgrind report in each of these cases?

**Answer:**

| Case            | Data Race | Helgrind                        |
| --------------- | --------- | ------------------------------- |
| remove 1 update | no race   | no data race errors             |
| lock 1 update   | race      | data race with lock on 1 thread |
| lock both       | no race   | no data race errors             |

---

3. Now let’s look at `main-deadlock.c`. Examine the code. This code has a problem known as deadlock (which we discuss in much more depth in a forthcoming chapter). Can you see what problem it might have?

**Answer:**

- Thread 0 wants to acquire locks in order `m1` -> `m2`
- Thread 1 wants to acquire locks in order `m2` -> `m1`
- Deadlock happens when:
  - Thread 0: acquire lock `m1`
  - Thread 1: acquire lock `m2`
  - Thread 0: trying to acquire lock `m2` but can't since it's acquired by Thread 1 so Thread 0 waits for Thread 1 to release the `m2`
  - Thread 1: trying to acquire lock `m1` but can't since it's acquired by Thread 0 so Thread 1 waits for Thread 0 to release the `m1`
  - Reasult: Thread 0 & Thread 1 wait for each other indefinitely

---

4. Now run helgrind on this code. What does helgrind report?

**Answer:** Lock Order Violation (which leads to deadlock)

```txt
==27504==
==27504== Thread #3: lock order "0x10C040 before 0x10C080" violated
==27504==
==27504== Observed (incorrect) order is: acquisition of lock at 0x10C080
==27504==    at 0x48512DC: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x109288: worker (main-deadlock.c:13)
==27504==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x4910AA3: start_thread (pthread_create.c:447)
==27504==    by 0x499DA63: clone (clone.S:100)
==27504==
==27504==  followed by a later acquisition of lock at 0x10C040
==27504==    at 0x48512DC: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x1092C3: worker (main-deadlock.c:14)
==27504==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x4910AA3: start_thread (pthread_create.c:447)
==27504==    by 0x499DA63: clone (clone.S:100)
==27504==
==27504== Required order was established by acquisition of lock at 0x10C040
==27504==    at 0x48512DC: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x10920E: worker (main-deadlock.c:10)
==27504==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x4910AA3: start_thread (pthread_create.c:447)
==27504==    by 0x499DA63: clone (clone.S:100)
==27504==
==27504==  followed by a later acquisition of lock at 0x10C080
==27504==    at 0x48512DC: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x109249: worker (main-deadlock.c:11)
==27504==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x4910AA3: start_thread (pthread_create.c:447)
==27504==    by 0x499DA63: clone (clone.S:100)
==27504==
==27504==  Lock at 0x10C040 was first observed
==27504==    at 0x48512DC: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x10920E: worker (main-deadlock.c:10)
==27504==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x4910AA3: start_thread (pthread_create.c:447)
==27504==    by 0x499DA63: clone (clone.S:100)
==27504==  Address 0x10c040 is 0 bytes inside data symbol "m1"
==27504==
==27504==  Lock at 0x10C080 was first observed
==27504==    at 0x48512DC: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x109249: worker (main-deadlock.c:11)
==27504==    by 0x4854B7A: ??? (in /usr/libexec/valgrind/vgpreload_helgrind-amd64-linux.so)
==27504==    by 0x4910AA3: start_thread (pthread_create.c:447)
==27504==    by 0x499DA63: clone (clone.S:100)
==27504==  Address 0x10c080 is 0 bytes inside data symbol "m2"
==27504==
==27504==
==27504==
==27504== Use --history-level=approx or =none to gain increased speed, at
==27504== the cost of reduced accuracy of conflicting-access information
==27504== For lists of detected and suppressed errors, rerun with: -s
==27504== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 8 from 8)
```

---

5. Now run helgrind on `main-deadlock-global.c`. Examine the code; does it have the same problem that `main-deadlock.c` has? Should helgrind be reporting the same error? What does this tell you about tools like helgrind?

**Answer:**

- this code doesn't have deadlock problem since they start with the same lock.
- helgrind shows the same Lock order violation error
- helgrind probably check for some rules that guarantees correctness like lock acquisition order as a good practice. But that means that there might be other ways that are correct but reported as errors

---

6. Let’s next look at `main-signal.c`. This code uses a variable (done) to signal that the child is done and that the parent can now continue. Why is this code inefficient? (what does the parent end up spending its time doing, particularly if the child thread takes a long time to complete?)

**Answer:** The code is inefficient since the parent is spinning doing nothing, wasting CPU cycles while it can sleep until the done variable change then wake up to continue

---

7. Now run helgrind on this program. What does it report? Is the code correct?

**Answer:** helgrind reports possible data race. I guess the code is correct there is nothing wrong with it.

---

8. Now look at a slightly modified version of the code, which is found in `main-signal-cv.c`. This version uses a condition variable to do the signaling (and associated lock). Why is this code preferred to the previous version? Is it correctness, or performance, or both?

**Answer:** The code is better due to:

- Performance: the parent sleeps until the child sends a signal that it's done. the saves wasted CPU cycles
- Correctness: I guess there is no difference , but I would opt for CV as a more reliable and methodical way to write correct code

---

9. Once again run helgrind on `main-signal-cv`. Does it report any errors?

**Answer:** No, helgrind doesn't report any error

---
