# An xv6 Lottery Scheduler

This project extends the **xv6** operating system with a **lottery scheduler**. It allows switching between **Round Robin (RR)** and **Lottery Scheduling (LS)** policies at runtime, and provides user programs to observe and test scheduling behavior.

---

## Instructions

The class requirements can be found [here](./instructions.md).

---

## Build & Run

From the `src` directory:

```bash
cd ./src
make qemu-nox
```

This will build xv6 and start QEMU in no-X mode.

---

## User Programs

- `loop N` — runs a CPU-bound process with **N tickets** (infinite loop).
- `ps` — prints scheduler statistics for all processes.
- `ps r` — resets CPU counters, then shows process stats.
- `ps set <p>` — sets the scheduling policy:

  - `0` → Round Robin (RR)
  - `1` → Lottery (LS)

---

## Testing the Scheduler

1. **Set policy to Round Robin**

   ```bash
   ps set 0
   ```

2. **Launch multiple processes**

   ```bash
   loop 10 &; loop 20 &; loop 30 &
   ```

3. **Check process statistics**

   ```bash
   ps
   ```

   → CPU usage should be roughly equal (~33% each).

4. **Switch to Lottery Scheduling**

   ```bash
   ps set 1
   ```

5. **Check process statistics again**

   ```bash
   ps
   ```

   → CPU usage should approximate the ticket distribution (10:20:30).

---

## Expected Results

- **Round Robin:**
  Each process gets ~33% CPU time, regardless of tickets.

- **Lottery Scheduling:**
  CPU time share is proportional to ticket count (e.g., ~16%, ~33%, ~50% for 10, 20, 30 tickets), with small variations (1–2%).
