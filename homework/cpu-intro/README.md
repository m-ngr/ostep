# CPU Intro Homework

- Homework [Instructions](./instructions.md)

## Questions

1. Run [process-run.py](./process-run.py) with the following flags: `-l 5:100,5:100`. What should the CPU utilization be (e.g., the percent of time the CPU is in use?) Why do you know this? Use the -c and -p flags to see if you were right.

**Answer:**
The CPU utilization will be 100% because we are running 2 processes, each process runs 5 CPU instructions

---

2. Now run with these flags: [./process-run.py](./process-run.py) `-l 4:100,1:0`. These flags specify one process with 4 instructions (all to use the CPU), and one that simply issues an I/O and waits for it to be done. How long does it take to complete both processes? Use -c and -p to find out if you were right.

**Answer:** Total cycles are 11 for the 2 processes to complete.

| Time | Process 1 | Process 2 | Comment                                |
| ---- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     |                                        |
| 2    | Running   | Ready     |                                        |
| 3    | Running   | Ready     |                                        |
| 4    | Running   | Ready     | End of Process 1                       |
| 5    | Done      | Running   | I/O Syscall                            |
| 6    | Done      | Blocked   |                                        |
| 7    | Done      | Blocked   |                                        |
| 8    | Done      | Blocked   |                                        |
| 9    | Done      | Blocked   |                                        |
| 10   | Done      | Blocked   |                                        |
| 11   | Done      | Running   | I/O Syscall Returns (End of Process 2) |

---

3. Switch the order of the processes: `-l 1:0,4:100`. What happens now? Does switching the order matter? Why? (As always, use -c and -p to see if you were right)

**Answer:** Total cycles are 7 for the 2 processes to complete.

The system will switch to process 2 while process 1 waits the I/O to complete, hence improve CPU utilization and reduces cycles needed

| Time | Process 1 | Process 2 | Comment                                |
| ---- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     | I/O Syscall                            |
| 2    | Blocked   | Running   |                                        |
| 3    | Blocked   | Running   |                                        |
| 4    | Blocked   | Running   |                                        |
| 5    | Blocked   | Running   | End of process 2                       |
| 6    | Blocked   | Done      |                                        |
| 7    | Running   | Done      | I/O Syscall Returns (End of Process 1) |

---

4. We’ll now explore some of the other flags. One important flag is -S, which determines how the system reacts when a process issues an I/O. With the flag set to `SWITCH_ON_END`, the system will NOT switch to another process while one is doing I/O, instead waiting until the process is completely finished. What happens when you run the following two processes (`-l 1:0,4:100 -c -S SWITCH_ON_END`), one doing I/O and the other doing CPU work?

**Answer:** Total cycles are 11 for the 2 processes to complete.

When the system doesn't switch processes, it reduces CPU utilization by leaving it idle while waiting for I/O operations to finish

| Time | Process 1 | Process 2 | Comment                                |
| ---- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     | I/O Syscall                            |
| 2    | Blocked   | Ready     |                                        |
| 3    | Blocked   | Ready     |                                        |
| 4    | Blocked   | Ready     |                                        |
| 5    | Blocked   | Ready     |                                        |
| 6    | Blocked   | Ready     |                                        |
| 7    | Running   | Ready     | I/O Syscall Returns (End of Process 1) |
| 8    | Done      | Running   |                                        |
| 9    | Done      | Running   |                                        |
| 10   | Done      | Running   |                                        |
| 11   | Done      | Running   | End of Process 2                       |

---

5. Now, run the same processes, but with the switching behavior set to switch to another process whenever one is WAITING for I/O (`-l 1:0,4:100 -c -S SWITCH_ON_IO`). What happens now? Use -c and -p to confirm that you are right.

**Answer:** Total cycles are 7 for the 2 processes to complete.

The system will switch to process 2 while process 1 waits the I/O to complete, hence improve CPU utilization and reduces cycles needed

| Time | Process 1 | Process 2 | Comment                                |
| ---- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     | I/O Syscall                            |
| 2    | Blocked   | Running   |                                        |
| 3    | Blocked   | Running   |                                        |
| 4    | Blocked   | Running   |                                        |
| 5    | Blocked   | Running   | End of process 2                       |
| 6    | Blocked   | Done      |                                        |
| 7    | Running   | Done      | I/O Syscall Returns (End of Process 1) |

---

6. One other important behavior is what to do when an I/O completes. With `-I IO_RUN_LATER`, when an I/O completes, the process that issued it is not necessarily run right away; rather, whatever was running at the time keeps running. What happens when you run this combination of processes? (`./process-run.py -l 3:0,5:100,5:100,5:100 -S SWITCH_ON_IO -c -p -I IO_RUN_LATER`) Are system resources being effectively utilized?

**Answer:** Total cycles are 31 for the 4 processes to complete.

In that specific case it would be better to run Process 1 after I/O finishes directly, it would result in a better CPU utilization

| Time | Process 1 | Process 2 | Process 3 | Process 4 | Comment                                |
| ---- | --------- | --------- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     | Ready     | Ready     | I/O Syscall                            |
| 2    | Blocked   | Running   | Ready     | Ready     |                                        |
| 3    | Blocked   | Running   | Ready     | Ready     |                                        |
| 4    | Blocked   | Running   | Ready     | Ready     |                                        |
| 5    | Blocked   | Running   | Ready     | Ready     |                                        |
| 6    | Blocked   | Running   | Ready     | Ready     | End of Process 2                       |
| 7    | Ready     | Done      | Running   | Ready     | I/O Syscall Ended                      |
| 8    | Ready     | Done      | Running   | Ready     |                                        |
| 9    | Ready     | Done      | Running   | Ready     |                                        |
| 10   | Ready     | Done      | Running   | Ready     |                                        |
| 11   | Ready     | Done      | Running   | Ready     | End of Process 3                       |
| 12   | Ready     | Done      | Done      | Running   |                                        |
| 13   | Ready     | Done      | Done      | Running   |                                        |
| 14   | Ready     | Done      | Done      | Running   |                                        |
| 15   | Ready     | Done      | Done      | Running   |                                        |
| 16   | Ready     | Done      | Done      | Running   | End of Process 4                       |
| 17   | Running   | Done      | Done      | Done      | I/O Syscall Returns                    |
| 18   | Running   | Done      | Done      | Done      | I/O Syscall                            |
| 19   | Blocked   | Done      | Done      | Done      |                                        |
| 20   | Blocked   | Done      | Done      | Done      |                                        |
| 21   | Blocked   | Done      | Done      | Done      |                                        |
| 22   | Blocked   | Done      | Done      | Done      |                                        |
| 23   | Blocked   | Done      | Done      | Done      |                                        |
| 24   | Running   | Done      | Done      | Done      | I/O Syscall Returns                    |
| 25   | Running   | Done      | Done      | Done      | I/O Syscall                            |
| 26   | Blocked   | Done      | Done      | Done      |                                        |
| 27   | Blocked   | Done      | Done      | Done      |                                        |
| 28   | Blocked   | Done      | Done      | Done      |                                        |
| 29   | Blocked   | Done      | Done      | Done      |                                        |
| 30   | Blocked   | Done      | Done      | Done      |                                        |
| 31   | Running   | Done      | Done      | Done      | I/O Syscall Returns (End of Process 1) |

---

7. Now run the same processes, but with `-I IO_RUN_IMMEDIATE` set, which immediately runs the process that issued the I/O. How does this behavior differ? Why might running a process that just completed an I/O again be a good idea?

**Answer:** Total cycles are 21 for the 4 processes to complete.

| Time | Process 1 | Process 2 | Process 3 | Process 4 | Comment                                |
| ---- | --------- | --------- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     | Ready     | Ready     | I/O Syscall                            |
| 2    | Blocked   | Running   | Ready     | Ready     |                                        |
| 3    | Blocked   | Running   | Ready     | Ready     |                                        |
| 4    | Blocked   | Running   | Ready     | Ready     |                                        |
| 5    | Blocked   | Running   | Ready     | Ready     |                                        |
| 6    | Blocked   | Running   | Ready     | Ready     | End of Process 2                       |
| 7    | Running   | Done      | Ready     | Ready     | I/O Syscall Returns                    |
| 8    | Running   | Done      | Ready     | Ready     | I/O Syscall                            |
| 9    | Blocked   | Done      | Running   | Ready     |                                        |
| 10   | Blocked   | Done      | Running   | Ready     |                                        |
| 11   | Blocked   | Done      | Running   | Ready     |                                        |
| 12   | Blocked   | Done      | Running   | Ready     |                                        |
| 13   | Blocked   | Done      | Running   | Ready     | End of Process 3                       |
| 14   | Running   | Done      | Done      | Ready     | I/O Syscall Returns                    |
| 15   | Running   | Done      | Done      | Ready     | I/O Syscall                            |
| 16   | Blocked   | Done      | Done      | Running   |                                        |
| 17   | Blocked   | Done      | Done      | Running   |                                        |
| 18   | Blocked   | Done      | Done      | Running   |                                        |
| 19   | Blocked   | Done      | Done      | Running   |                                        |
| 20   | Blocked   | Done      | Done      | Running   | End of Process 4                       |
| 21   | Running   | Done      | Done      | Done      | I/O Syscall Returns (End of Process 1) |

**In that specific case:** it would be better to run Process 1 after I/O finishes directly, it would result in a better CPU utilization

**The counter case:** let say we have process 1 (io, cpu x6) and process 2 (cpu x6, io)

- M1: `IO_RUN_LATER` (takes 16 cycles)
- M2: `IO_RUN_IMMEDIATE` (takes 21 cycles)

| Time | M1:P1      | M1:P2      | M2:P1      | M2:P2      |
| ---- | ---------- | ---------- | ---------- | ---------- |
| 1    | Run:IO     | Ready      | Run:IO     | Ready      |
| 2    | Blocked    | Run:CPU_1  | Blocked    | Run:CPU_1  |
| 3    | Blocked    | Run:CPU_2  | Blocked    | Run:CPU_2  |
| 4    | Blocked    | Run:CPU_3  | Blocked    | Run:CPU_3  |
| 5    | Blocked    | Run:CPU_4  | Blocked    | Run:CPU_4  |
| 6    | Blocked    | Run:CPU_5  | Blocked    | Run:CPU_5  |
| 7    | Ready      | Run:CPU_6  | Run:IO_END | Ready      |
| 8    | Ready      | Run:IO     | Run:CPU_1  | Ready      |
| 9    | Run:IO_END | Blocked    | Run:CPU_2  | Ready      |
| 10   | Run:CPU_1  | Blocked    | Run:CPU_3  | Ready      |
| 11   | Run:CPU_2  | Blocked    | Run:CPU_4  | Ready      |
| 12   | Run:CPU_3  | Blocked    | Run:CPU_5  | Ready      |
| 13   | Run:CPU_4  | Blocked    | Run:CPU_6  | Ready      |
| 14   | Run:CPU_5  | Ready      | Done       | Run:CPU_6  |
| 15   | Run:CPU_6  | Ready      | Done       | Run:IO     |
| 16   | Done       | Run:IO_END | Done       | Blocked    |
| 17   | Done       | Done       | Done       | Blocked    |
| 18   | Done       | Done       | Done       | Blocked    |
| 19   | Done       | Done       | Done       | Blocked    |
| 20   | Done       | Done       | Done       | Blocked    |
| 21   | Done       | Done       | Done       | Run:IO_END |

**The general case:** it might be a good idea to actually re-run the process that issued an I/O when finished, there is a higher chance that this process will issue another I/O operation (which gives a room for more CPU untilization)

---

8. Now run with some randomly generated processes using flags `-s 1 -l 3:50,3:50` or `-s 2 -l 3:50,3:50` or `-s 3 -l 3:50, 3:50`. See if you can predict how the trace will turn out. What happens when you use the flag `-I IO_RUN_IMMEDIATE` versus that flag `-I IO_RUN_LATER`? What happens when you use the flag `-S SWITCH_ON_IO` versus `-S SWITCH_ON_END`?

**Answer:**

### Part 1

Args: `-s 1 -l 3:50,3:50`
System behaviors: `SWITCH_ON_IO` && `IO_RUN_LATER`

| Time | Process 1 | Process 2 | Comment                                |
| ---- | --------- | --------- | -------------------------------------- |
| 1    | Running   | Ready     |                                        |
| 2    | Running   | Ready     | I/O Syscall                            |
| 3    | Blocked   | Running   |                                        |
| 4    | Blocked   | Running   |                                        |
| 5    | Blocked   | Running   | End of Process 2                       |
| 6    | Blocked   | Done      |                                        |
| 7    | Blocked   | Done      |                                        |
| 8    | Running   | Done      | I/O Syscall returns                    |
| 9    | Running   | Done      | I/O Syscall                            |
| 10   | Blocked   | Done      |                                        |
| 11   | Blocked   | Done      |                                        |
| 12   | Blocked   | Done      |                                        |
| 13   | Blocked   | Done      |                                        |
| 14   | Blocked   | Done      |                                        |
| 15   | Running   | Done      | I/O Syscall returns (End of Process 1) |

**Notes:**

- Running with `-S SWITCH_ON_END` will reduce CPU utilization by adding 3 more cycles for process 2 at the end while leaving the CPU idle at cycles 3:5
- Running with `-I IO_RUN_IMMEDIATE` won't have any impact because all other processes are done while waiting for the first I/O to finish

### Part 2

Args: `-s 2 -l 3:50,3:50`
System behaviors: `SWITCH_ON_IO` && `IO_RUN_LATER`

| Time | Process 1 | Process 2 | Comment                                    |
| ---- | --------- | --------- | ------------------------------------------ |
| 1    | Running   | Ready     | P1: I/O Syscall                            |
| 2    | Blocked   | Running   |                                            |
| 3    | Blocked   | Running   | P2: I/O Syscall                            |
| 4    | Blocked   | Blocked   |                                            |
| 5    | Blocked   | Blocked   |                                            |
| 6    | Blocked   | Blocked   |                                            |
| 7    | Running   | Blocked   | P1: I/O Syscall returns                    |
| 8    | Running   | Blocked   | P1: I/O Syscall                            |
| 9    | Blocked   | Running   | P2: I/O Syscall returns                    |
| 10   | Blocked   | Running   | P2: I/O Syscall                            |
| 11   | Blocked   | Blocked   |                                            |
| 12   | Blocked   | Blocked   |                                            |
| 13   | Blocked   | Blocked   |                                            |
| 14   | Running   | Blocked   | P1: I/O Syscall returns                    |
| 15   | Running   | Blocked   | End of Process 1                           |
| 16   | Done      | Running   | P2: I/O Syscall returns (End of Process 2) |

**Notes:**

- Running with `-S SWITCH_ON_END` will reduce CPU utilization by adding 14 more cycles for process 2 at the end while leaving the CPU idle at cycles (2,3,9,10)
- Running with `-I IO_RUN_IMMEDIATE` won't have any impact

### Part 3

Args: `-s 3 -l 3:50,3:50`
System behaviors: `SWITCH_ON_IO` && `IO_RUN_LATER`

| Time | Process 1 | Process 2 | Comment                 |
| ---- | --------- | --------- | ----------------------- |
| 1    | Running   | Ready     |                         |
| 2    | Running   | Ready     | P1: I/O Syscall         |
| 3    | Blocked   | Running   | P2: I/O Syscall         |
| 4    | Blocked   | Blocked   |                         |
| 5    | Blocked   | Blocked   |                         |
| 6    | Blocked   | Blocked   |                         |
| 7    | Blocked   | Blocked   |                         |
| 8    | Running   | Blocked   | P1: I/O Syscall returns |
| 9    | Running   | Ready     | End of Process 1        |
| 10   | Done      | Running   | P2: I/O Syscall returns |
| 11   | Done      | Running   | P2: I/O Syscall         |
| 12   | Done      | Blocked   |                         |
| 13   | Done      | Blocked   |                         |
| 14   | Done      | Blocked   |                         |
| 15   | Done      | Blocked   |                         |
| 16   | Done      | Blocked   |                         |
| 17   | Done      | Running   | P2: I/O Syscall returns |
| 18   | Done      | Running   | End of Process 2        |

**Notes:**

- Running with `-S SWITCH_ON_END` will reduce CPU utilization by adding 6 more cycles for process 2 at the end while leaving the CPU idle at cycle (3)
- Running with `-I IO_RUN_IMMEDIATE` , Process 2 at cycle 9 will run instead of process 1, hence it will save a cycle (total running cycles will be 17)
