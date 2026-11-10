# 598APE-HW3: optimized abelian sandpile

This fork speeds up the identity sandpile of the 1001 x 1001 pacman grid with 7 optimizations, one commit each.
Every commit writes a PNG that is byte-identical to the one of the baseline, at every grid size and thread count we tried.
No optimization needs special hardware beyond an x86-64 processor with AVX2 (any since 2013); `make ARCH=` builds without it.
The original course README follows below.

- **Tools:**
  everything runs in the container built from `docker/Dockerfile` (Ubuntu 24.04, GCC 13.3 with OpenMP, git, bc, perf and matplotlib).
  `./dockerrun.sh` builds the image on first use and opens a shell with this repository at `/host`, where the commands below run.
- **Build and run:**
  `make all`, then `./sandpile.exe 1001`, which writes `identity.png`.
  `OMP_NUM_THREADS` sets the thread count from `opt7` on.
- **Reproduce:**
  `test/run.sh` builds every commit below into `test/build/<name>` and times it at N = 1001, three runs each on one thread, appending to `test/results/times.csv`;
  `test/run.sh opt5 opt6` times only those commits, and `N`, `RUNS` and `THREADS` change the grid size, the run count and the thread count.
  `test/check.sh` checks that every commit writes the same PNG as `base` at 15 small grid sizes, odd and even, and at N = 1001 if `test/run.sh` saved both PNGs.
  `test/bench.sh` collects every time in the report: all commits pinned to one core, then `opt7` on 2 to 32 cores with `OMP_PROC_BIND=close OMP_PLACES=cores`.
  The baseline takes about 5.5 minutes per run, so the whole script takes about an hour.
- **Figures:**
  `python3 plots/scripts/plot.py` draws the figures of the report from the files in `plots/data/`.

| Name   | Commit    | Change                                                       | Report |
| ------ | --------- | ------------------------------------------------------------ | ------ |
| `base` | `15fe837` | unmodified course code                                       |        |
| `opt1` | `d812736` | precompute the column range of each row                      | 2.1    |
| `opt2` | `768d7ce` | one contiguous grid with a ghost border                      | 2.1    |
| `opt3` | `661956c` | topple a cell as many times as it can at once                | 2.1    |
| `opt4` | `6997e3e` | topple all cells at once on a byte grid, vectorized          | 2.2    |
| `opt5` | `c76af1a` | update only the upper half for odd N, by symmetry            | 2.3    |
| `opt6` | `635308b` | compile for x86-64-v3 (AVX2)                                 | 2.4    |
| `opt7` | `92be6c9` | parallelize the steps with OpenMP                            | 2.5    |

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

