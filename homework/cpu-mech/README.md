# Homework 3: Measuring System Call and Context Switch Costs

This homework explores the performance costs of **system calls** and **context switches** on a Linux system. It includes programs to measure these costs and report them in nanoseconds.

## System Specifications

- **CPU:** 12th Gen Intel(R) Core(TM) i7-1255U, 10 cores, 12 threads (2 threads per core)
- **Architecture:** x86_64, 64-bit
- **CPU Frequency:** 400 MHz (min) – 4.7 GHz (max)
- **Cache:**
  - L1d: 352 KiB (10 instances)
  - L1i: 576 KiB (10 instances)
  - L2: 6.5 MiB (4 instances)
  - L3: 12 MiB (1 instance)
- **Memory:** 15 GiB RAM, 14 GiB swap
- **NUMA Nodes:** 1 (CPUs 0–11)
- **Virtualization:** VT-x supported
- **Operating System:** Ubuntu 24.04.3 LTS
- **Kernel:** Linux 6.14.0-28-generic, SMP, PREEMPT_DYNAMIC
- **Clock Source:** `CLOCK_MONOTONIC_RAW` used for high-resolution timing

> **Note:** All benchmark results in this report were measured on this system. Performance may vary on other hardware or operating systems.

---

## 1. Measuring System Calls

There are 2 files:

- [`syscall_v1.c`](./syscall_v1.c): Uses `gettimeofday()` to measure time in **microseconds**.
- [`syscall_v2.c`](./syscall_v2.c): Uses `clock_gettime(CLOCK_MONOTONIC_RAW)` for **nanosecond resolution**.

### Build

```bash
gcc syscall_v2.c -o syscall -Wall -Werror
```

### Usage

```bash
./syscall [function-name] [calls-per-round] [rounds]
```

Available function names: `getpid`, `getppid`, `getuid`, `read`.

Defaults:

- `function-name`: `read`
- `calls-per-round`: `10000000` (10M)
- `rounds`: `10`

---

### Example Output: `read`

```bash
./syscall read
```

```txt
[001] diff = 1699158499 nsec
[002] diff = 1700919721 nsec
[003] diff = 1683075704 nsec
[004] diff = 1681516906 nsec
[005] diff = 1701268968 nsec
[006] diff = 1670595604 nsec
[007] diff = 1688329618 nsec
[008] diff = 1672862443 nsec
[009] diff = 1662375600 nsec
[010] diff = 1676040727 nsec
------------------------------
[AVG] diff = 1683614379 nsec
==============================
AvgSysCall = 168.361438 nsec
```

---

### Example Output: `getpid`

```bash
./syscall getpid
```

```txt
[001] diff = 695287725 nsec
[002] diff = 701925233 nsec
[003] diff = 695532219 nsec
[004] diff = 704200939 nsec
[005] diff = 695802901 nsec
[006] diff = 688948432 nsec
[007] diff = 687258972 nsec
[008] diff = 687563021 nsec
[009] diff = 690983100 nsec
[010] diff = 694994789 nsec
------------------------------
[AVG] diff = 694249733 nsec
==============================
AvgSysCall = 69.424973 nsec
```

---

## 2. Measuring Context Switch

### Build

```bash
gcc context_switch.c -o ctxswtich -Wall -Werror
```

### Usage

```bash
./ctxswtich [loops?]
```

Defaults:

- `loops`: `1000000` (1M)

### Example Output

```bash
./ctxswtich
```

```txt
AVG Context Switch Time = 1103.002372 nsec
```

---

## 3. Comparing Results

| Benchmark                | Average Time (ns) |
| ------------------------ | ----------------- |
| Light syscall (`getpid`) | \~69              |
| Heavy syscall (`read`)   | \~168             |
| Context switch           | \~1103            |

- **Observation:** The cost of a context switch is roughly 6.5× higher than a heavy syscall (read).
- This demonstrates that context switching is more expensive than performing even a relatively heavy system call.
