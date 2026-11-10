# 598APE-HW3: optimized abelian sandpile

This fork speeds up the identity sandpile of the 1001 x 1001 pacman grid in 7 commits, one optimization each.
Every commit writes the same PNG as the baseline, byte for byte, at every grid size and thread count we tried.
The original course README follows below.

| Name   | Commit    | Change                                              | Report | 1 thread |
| ------ | --------- | --------------------------------------------------- | ------ | -------- |
| `base` | `15fe837` | unmodified course code                              |        | 343 s    |
| `opt1` | `d812736` | precompute the column range of each row             | 2.1    | 171 s    |
| `opt2` | `768d7ce` | one contiguous grid with a ghost border             | 2.1    | 179 s    |
| `opt3` | `661956c` | topple a cell as many times as it can at once       | 2.1    | 160 s    |
| `opt4` | `6997e3e` | topple all cells at once on a byte grid, vectorized | 2.2    | 26.8 s   |
| `opt5` | `c76af1a` | update only the upper half for odd N, by symmetry   | 2.3    | 13.4 s   |
| `opt6` | `635308b` | compile for x86-64-v3 (AVX2)                        | 2.4    | 10.1 s   |
| `opt7` | `92be6c9` | parallelize the steps with OpenMP                   | 2.5    | 10.3 s   |

`opt7` takes 1.03 s on 24 threads. Times are medians of three runs on an AMD EPYC 9334 server.

## Requirements

- Linux on an x86-64 processor with AVX2. `make ARCH=` builds without AVX2.
- A C compiler with OpenMP, e.g. GCC, and make.
- For the scripts: git, bc and taskset. For the figures: Python 3 with matplotlib.

## Steps

Run every command from the repository root.

1. **Build and run.**
   ```bash
   make all
   OMP_NUM_THREADS=16 ./sandpile.exe 1001
   ```
   This writes `identity.png`.
2. **Check the outputs.**
   `test/check.sh` builds each commit of `test/commits.txt` into `test/build/<name>` and checks that it writes the same PNG as `base` at 15 small grid sizes, odd and even.
   It prints `optN: same as base` for each commit.
3. **Time every commit.**
   `test/bench.sh` times every commit three times on one core, then `opt7` on 2, 4, 8, 16, 24 and 32 threads with `OMP_PROC_BIND=close OMP_PLACES=cores`, and then runs `test/check.sh`.
   The times go to `test/results/times.csv`.
   It takes about an hour, most of it in the baseline.
   To time only some commits, run e.g. `test/run.sh opt5 opt6`; `N`, `RUNS` and `THREADS` set the grid size, the run count and the thread count.
4. **Draw the figures.**
   ```bash
   cp test/results/times.csv plots/data/
   python3 plots/scripts/plot.py
   ```
   This writes `plots/figures/commits.pdf` and `plots/figures/steps_threads.pdf`.
   `plots/data/steps.csv` holds the sweep counts of `opt1` and the step counts of `opt4`, which we counted with a counter in `stabilize()`.
5. **Larger grids (optional challenge).**
   ```bash
   OMP_NUM_THREADS=32 OMP_PROC_BIND=close OMP_PLACES=cores ./sandpile.exe 2001
   ```

---

# CS598APE HW3

This repository contains code for homework 3 of CS598APE. It is relatively
simple in comparison to HW1 and HW2 to ensure you have enough time to work on
the course project. 

`sandpile.c` contains the implementation of finding the identity element of a 2D
abelian sandpile.

![Identity sandpile on a 1001x1001 pacman grid](identity-ref.png)

*Identity sandpile on a 1001x1001 pacman grid*


To compile the program, run:

```bash
make all
```

To clean existing build artifacts, run:

```bash
make clean
```

Usage (after compiling):

```bash
./sandpile.exe <N> 
```

where `N` is the grid size

Consider speeding up the generation of this 1001x1001 identity sandpile (which
takes approximately ~3m 40s on my local laptop under the default setup)

```bash
./sandpile.exe 1001
```

For an optional, ungraded challenge, see how large a sandpile you can compute
on the course VMs and report the grid size and runtime. There is no prescribed
upper limit on `N` for this challenge.

