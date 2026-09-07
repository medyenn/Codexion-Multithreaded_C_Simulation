*This project has been created as part of the 42 curriculum by mohael-g.*

# Codexion — Master the race for resources before the deadline masters you

## Description

**Codexion** is a concurrency simulation written in C using POSIX threads.
It models a circular co-working hub where **N coders** share **N USB dongles**
placed between adjacent seats.

- A coder needs **two dongles simultaneously** (left + right) to compile quantum code.
- After compiling, the coder debugs, then refactors, then tries to compile again.
- If a coder fails to begin a new compile within `time_to_burnout` ms since their
  last compile start, they **burn out** and the simulation ends.
- The simulation also ends cleanly when every coder has compiled at least
  `number_of_compiles_required` times.

Goals: prevent deadlocks, starvation, and data races while enforcing fair
dongle scheduling (FIFO or EDF) with per-dongle cooldown periods.

---

## Instructions

### Compilation

```bash
make          # builds ./codexion
make re       # full rebuild
make clean    # remove object files
make fclean   # remove objects + binary
```

### Execution

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders (threads) and dongles |
| `time_to_burnout` | Max ms a coder may go without starting a compile |
| `time_to_compile` | Duration of the compile phase (ms) |
| `time_to_debug` | Duration of the debug phase (ms) |
| `time_to_refactor` | Duration of the refactor phase (ms) |
| `number_of_compiles_required` | Stop when all coders reach this count (0 = immediate stop) |
| `dongle_cooldown` | Ms a dongle is unavailable after being released |
| `scheduler` | `fifo` (arrival order) or `edf` (earliest deadline first) |

### Examples

```bash
# 1 coder, easy — should complete 3 compiles without burning out
./codexion 1 800 200 200 200 3 0 fifo

# 4 coders, FIFO, no cooldown
./codexion 4 800 200 200 200 5 0 fifo

# 4 coders, EDF, 50 ms cooldown
./codexion 4 800 200 200 200 5 50 edf

# Tight timing — should survive without burnout
./codexion 5 310 200 100 100 5 0 fifo

# Invalid args — must print error and exit 1
./codexion -1 800 200 200 200 5 0 fifo
./codexion 4 800 200 200 200 5 0 random
```

---

## Blocking cases handled

### Deadlock Prevention (Coffman's conditions)

The four Coffman conditions for deadlock are:
1. **Mutual exclusion** — yes, each dongle has a mutex.
2. **Hold and wait** — a coder holds a dongle while waiting for the second.
3. **No preemption** — dongles are voluntarily released after compile.
4. **Circular wait** — **broken** by always acquiring dongles in ascending `id` order.

Because every coder (except ties resolved by id) acquires the lower-id dongle
first, the cycle `A waits for B waits for A` can never form.

### Starvation Prevention

- **FIFO**: requests are served strictly in arrival order via a min-heap keyed on
  timestamp — no coder can be overtaken indefinitely.
- **EDF**: the coder with the nearest burnout deadline is served first.
  Because each coder's deadline advances with every compile, no coder is
  permanently deprioritised.
- A staggered start (even-numbered coders sleep `time_to_compile / 2` ms before
  their first attempt) prevents a thundering-herd at simulation start.

### Cooldown Handling

Each dongle records its `release_time`. A waiting coder checks
`now >= release_time + dongle_cooldown` inside its condition-variable loop before
marking the dongle as taken. If the cooldown has not elapsed the coder re-waits
with a 0.5 ms `pthread_cond_timedwait` timeout to avoid busy-polling.

### Precise Burnout Detection

A dedicated **monitor thread** polls every 0.5 ms
(`usleep(500)`). For each coder it computes
`now − last_compile_start` and triggers a stop if the value ≥ `time_to_burnout`.
The burnout timestamp is captured *before* acquiring the log mutex so the
printed delay is minimised to well under 10 ms.

### Log Serialisation

All `printf` calls are wrapped in `log_event()` / `log_burnout()`, which hold
`sim->log_mutex` for the duration of the write. This guarantees that no two
status lines ever interleave in the output.

---

## Thread synchronization mechanisms

### Primitives used

| Primitive | Purpose |
|---|---|
| `pthread_mutex_t log_mutex` | Serialise all stdout output |
| `pthread_mutex_t stop_mutex` | Atomic read/write of `sim->stopped` flag |
| `pthread_mutex_t dongle.mutex` | Protect the dongle's state and its heap |
| `pthread_mutex_t coder.cond_mutex` | Pair with `coder.cond` (POSIX requirement) |
| `pthread_cond_t coder.cond` | Per-coder condition variable; signalled on dongle release or sim stop |
| `pthread_cond_timedwait` | Used in dongle wait loop to handle cooldown wakeup without busy-waiting |

### Race condition prevention

- **Dongle state** (`in_use`, `release_time`, heap) — always accessed under
  `dongle.mutex`.  No coder reads these fields without holding the lock.
- **`compile_count`** — written only by its own thread; read by the monitor.
  On x86/arm64, 32-bit aligned reads are naturally atomic, but the 0.5 ms
  polling interval provides a safe practical window.
- **`last_compile_start`** — written only by the coder thread; read by the
  monitor.  Same reasoning as above.
- **`sim->stopped`** — always read/written through `sim_is_stopped()` /
  `sim_stop()` which hold `stop_mutex`.

### Monitor ↔ coder communication

When the monitor sets `stopped = true` it calls `wake_all_coders()`:
1. Broadcasts on every coder's `cond` — unblocks any `pthread_cond_timedwait`.
2. Briefly acquires and releases each dongle's mutex — ensures no coder is
   mid-lock when the broadcast fires.
3. Broadcasts again — catches coders that just entered the wait after step 1.

Coder threads check `sim_is_stopped()` at every state-machine boundary
(before acquiring dongles, after each sleep) and return cleanly.

---

## Resources

### Concurrency references

- W. Richard Stevens — *Advanced Programming in the UNIX Environment* (chapters on threads)
- Maurice J. Bach — *The Design of the UNIX Operating System*
- Dijkstra, E. W. (1965) — *Solution of a problem in concurrent programming control*
  (the original dining philosophers paper)
- POSIX threads programming: <https://hpc-tutorials.llnl.gov/posix/>
- GNU libc manual — `pthread_cond_timedwait`:
  <https://www.gnu.org/software/libc/manual/html_node/Waiting-with-Explicit-Clocks.html>
- Coffman conditions (Wikipedia):
  <https://en.wikipedia.org/wiki/Deadlock#Coffman_conditions>
- Earliest Deadline First scheduling (Wikipedia):
  <https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling>

### AI usage

AI was used for:
- **Design review**: verifying that the resource-ordering deadlock-prevention
  strategy was correctly applied to a circular layout.
- **Project Architecture**
- **Pthread Library Understanding**
