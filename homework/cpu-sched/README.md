# CPU Scheduling Homework

- Homework [Instructions](./instructions.md)

## Questions

1. Compute the response time and turnaround time when running three jobs of length 200 with the SJF and FIFO schedulers.

**Answer:**

Since all jobs have the same length (200), then the SJF and FIFO methods are similar (there is no short jobs).

_Exectuion Table:_

| Time | Running Job | Duration |
| ---- | ----------- | -------- |
| 0    | job-0       | 200      |
| 200  | job-1       | 200      |
| 400  | job-2       | 200      |
| 600  | (END)       | -        |

_Stats:_

| Job   | Turnaround Time | Response Time | Wait Time |
| ----- | --------------- | ------------- | --------- |
| job-0 | 200             | 0             | 0         |
| job-1 | 400             | 200           | 200       |
| job-2 | 600             | 400           | 400       |
| avg   | 400             | 200           | 200       |

---

2. Now do the same but with jobs of different lengths: 100, 200, and 300.

**Answer:**

In this specific case where jobs are naturally sorted from shortest to longest SJF and FIFO methods behave in the same way.

_Exectuion Table:_

| Time | Running Job | Duration |
| ---- | ----------- | -------- |
| 0    | job-0       | 100      |
| 100  | job-1       | 200      |
| 300  | job-2       | 300      |
| 600  | (END)       | -        |

_Stats:_

| Job   | Turnaround Time | Response Time | Wait Time |
| ----- | --------------- | ------------- | --------- |
| job-0 | 100             | 0             | 0         |
| job-1 | 300             | 100           | 100       |
| job-2 | 600             | 300           | 300       |
| avg   | 333.33          | 133.33        | 133.33    |

---

3. Now do the same, but also with the RR scheduler and a time-slice of 1.

**Answer:**

In a RR schedular with Ts=1 and jobs (100,200,300): jobs will spread out with execution blocks that looks like the following:

- B0: (job0, job1, job2), with a time of 3 and it repeats until job-0 finishes at **298** _(100 \* 3 - 2)_
- B1: (job1, job2), with a time of 2 and it repeats until job-1 finishes at **499** _(300 + 100 \* 2 - 1)_
- B2: (job2), with a time of 1 and it repeats until job-2 finishes at **600** _(500 + 100 \* 1)_

_Stats:_

| Job   | Turnaround Time | Response Time | Wait Time |
| ----- | --------------- | ------------- | --------- |
| job-0 | 298             | 0             | 198       |
| job-1 | 499             | 1             | 299       |
| job-2 | 600             | 2             | 300       |
| avg   | 465.67          | 1             | 265.67    |

---

4. For what types of workloads does SJF deliver the same turnaround times as FIFO?

**Answer:**

When jobs arrived in a natually sorted order from shortest to longest (including jobs of equal lengths).

---

5. For what types of workloads and quantum lengths does SJF deliver the same response times as RR?

**Answer:**

When:

- All jobs arrived in a natually sorted order from shortest to longest (including jobs of equal lengths)
- All jobs are shorter than or equal to the quantum length

---

6. What happens to response time with SJF as job lengths increase? Can you use the simulator to demonstrate the trend?

**Answer:**

The response time increases as job lengths increase, except in the case where the only job that increases was the longest job already

---

7. What happens to response time with RR as quantum lengths increase? Can you write an equation that gives the worst-case response time, given N jobs?

**Answer:**

The response time increases as quantum lengths increase, except in the case where quantum length is already longer than all jobs.

**Worst Case response time for N jobs:**

- Assume all jobs are longer than or equal to quantum time _Q_
- Assume all jobs arrive at the same time

  Response Time = `(N-1) * Q`

---
