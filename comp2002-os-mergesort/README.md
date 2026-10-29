                     parallel_mergesort
                            |
                            |
                 level       >=     cutoff ?
                                  
                  Yes              No
                   |                |
                   |                |
            my_mergesort         split
                            |             |
                            |             |
                         pthread       pthread
                           |              |
                    parallel_sort   parallel_sort
                           \              /
                            \            /
                           pthread_join
                                |
                              merge




## COMP2002 Operating Systems — Parallel Merge Sort

* Authors: Ziming Yu (Member1), Hang Yu (Member2), Stuart Vass (Member 3)
* Group name: Your Group Name

## Overview

This project implements merge sort in C using both sequential recursion and POSIX threads. The array is divided into smaller sections, sorted independently, and merged back together; a configurable cutoff controls how many levels use parallel execution.

## Manifest

- `Makefile` — Builds `test-mergesort` by default, supports `make unit-tests`, and provides a `clean` target.
- `mergesort.c` — Implements `merge()`, sequential `my_mergesort()`, parallel `parallel_mergesort()`, and `buildArgs()`.
- `mergesort.h` — Declares the sorting functions, thread argument structure, cutoff, and shared arrays `A` and `B`.
- `test-mergesort.c` — Generates a seeded random input, runs one sorting test, measures elapsed time, and checks whether the output is sorted.
- `unit-tests.c` — Runs automated tests over multiple input sizes and cutoff levels, followed by a repeated timing comparison. Runs some unit tests.
- `README` — Describes the project, build and usage instructions, testing, limitations, and reflections.

Generated executables, `.o` object files, and `.d` dependency files are build outputs rather than source files.

## Building the project

The project is intended for a Linux environment (including WSL) with GCC, GNU Make, and POSIX threads available.

From the project directory, build the main program:

```bash
make clean
make
```

Build the separate automated test program when needed:

```bash
make unit-tests
```

The main executable is `./test-mergesort`; the automated test executable is `./unit-tests`.

**Build note:** In the supplied Makefile, `make clean` removes the main executable and object/dependency files, but not the `unit-tests` executable. The current `test-mergesort` link recipe also uses `$?` instead of `$^`, which can cause an incremental relink to fail when only one object file is rebuilt (see **Known Bugs**).

## Implementation Steps

1. Review the starter code, including the global arrays `A` (input/output) and `B` (temporary merge storage).

2. Implement and review `merge()` to combine two already-sorted adjacent index ranges.

3. Implement and review `my_mergesort()` with a single-element base case, recursive splitting, and merging.

4. Implement `parallel_mergesort()` so each non-base recursion level creates two child threads, joins them, and then merges the sorted ranges.

5. Pass each child its own heap-allocated `struct argument` containing `left`, `right`, and `level`, and release these arguments after joining.

6. Use the cutoff to switch from parallel recursion to sequential merge sort.

7. Build and test the implementation with different input sizes and cutoff levels; review performance and remaining issues before submission.

## Features and usage

Run the program with:

```bash
./test-mergesort <input_size> <cutoff_level> <seed>
```

- **`input_size`** — Number of random integers to sort. The main program requires at least 2.
- **`cutoff_level`** — Maximum depth of parallel splitting. `0` runs sequentially; a positive value permits child threads while `level < cutoff` and the current range contains more than one element.
- **`seed`** — Seed for reproducible pseudorandom input. The generated values are in the range `0` to `999999`.

For example:

```bash
./test-mergesort 100000 0 42   # Sequential merge sort
./test-mergesort 100000 2 42   # Parallel merge sort with cutoff 2
```

The program allocates `A` and `B`, generates the input, executes the sort, verifies that the final array is in nondecreasing order, and prints elapsed sorting time in seconds. It reports an error and exits if the sorted-order check fails.

The sequential function recursively sorts the left and right halves before calling `merge()`. In the parallel function, the parent waits for both child threads using `pthread_join()` before merging, so concurrently executing branches operate on separate array ranges. The shared arrays do not require a single mutex for this partitioned merge-sort approach.

## Testing

### 1. Single-run correctness tests

Use `test-mergesort` to validate the same input with different sizes and cutoff values. Setting `cutoff = 0` isolates the sequential implementation.

The following five sequential configurations were run successfully:

| Input size | Cutoff | Seed | Result |
| ---: | ---: | ---: | --- |
| 2 | 0 | 42 | Passed |
| 3 | 0 | 42 | Passed |
| 10 | 0 | 42 | Passed |
| 1,000 | 0 | 42 | Passed |
| 100,000 | 0 | 42 | Passed |

The current source was also compiled and checked with `100000` elements, seed `42`, and cutoff values `1`, `2`, `3`, and `4`; each run passed the sorted-order check. These runs establish correctness for the listed configurations, not a performance advantage.

Example commands:

```bash
./test-mergesort 2 0 42
./test-mergesort 100000 0 42
./test-mergesort 100000 1 42
./test-mergesort 100000 2 42
./test-mergesort 100000 3 42
./test-mergesort 100000 4 42
```

### 2. Automated size/cutoff test matrix

To ensure our program is working properly, we have written an additional series of automatic tests in `unit-tests.c`. 

These tests can be run via `make unit-tests` or once complied `./unit-tests`.

The tests can be broken into the following catergories:
- Tests for buildArg()
- Tests for my_mergesort()
- Tests for parallel_mergesort()

Table 2.1: Tests for buildArg()

| Input | Quantity |
| Random Values | 10 |
| Zeroes | 1 |
| Negative Random Values | 10 |

Table 2.2: Tests for my_mergesort()

| Input | Quantity |
| Small arrays | 0 to 9, i++ |
| Medium arrays | 100 to 900, i+=100 |
| Large arrays & Odds, Evens | 1000 to 9000, i+=1001 |
| Very large arrays | 100,000 to 900,000, i+=100,000 |

Table 2.3: Tests for parallel_mergesort()

| Input | Quantity |
| ARRAY OF ZERO | 1 |
| ARRAY OF SMALL SIZES | 1 to 10, i++ |
| ARRAY OF MEDIUM SIZES + ODDS, EVENS | 100 to 1000, i+=101 |
| ARRAY OF LARGE SIZES | 10,000 to 100,000, i+=10,000 |
| ARRAY OF VERY LARGE SIZES | 100,000 to 1,000,000, i+=100,000 |


The supplied `unit-tests.c` contains a correctness test matrix using seed `1` and cutoff values 0 through 10. For each cutoff value it exercises:

| Category in `unit-tests.c` | Input sizes | Cases per cutoff |
| --- | --- | ---: |
| Zero-length input | `0` | 1 |
| Small arrays | `1` through `10` | 10 |
| Medium arrays, mixing odd/even sizes | `100, 201, 302, ..., 908` | 9 |
| Large arrays | `10000, 20000, ..., 100000` | 10 |
| Very large arrays | `100000, 200000, ..., 1000000` | 10 |
| **Total per cutoff** | | **40** |

This gives **200 correctness checks** across five cutoff levels. During source review, the correctness portion was compiled and run separately from the resource-intensive benchmarking portion; **200/200 checks passed** in that bounded verification run.

The automated test harness directly calls the sorting functions, so it can exercise sizes `0` and `1` even though the normal `test-mergesort` command-line program rejects sizes below `2`. In those cases, no merge is needed. This is an internal boundary test, not an example of valid normal command-line usage.

Both test programs use `check_if_sorted()`, which compares adjacent values and rejects any decrease. This verifies nondecreasing order but does **not**, on its own, prove that the output contains exactly the same multiset of elements as the input. The current automated matrix also uses a fixed seed rather than varying several seeds.

### 3. Performance test design

After the correctness matrix, `unit-tests.c` is configured to benchmark **100,000,000 elements**. It runs cutoff `0` as a baseline, then cutoffs `1` through `10`. Each configuration is executed **three times**, and its mean elapsed time is recorded. The comparison printed by the program is:

```text
relative time (%) = 100 × mean_parallel_time / mean_sequential_time
```

A smaller percentage means less time than the sequential baseline. For example, `50%` relative time corresponds to a speedup of approximately `2×`:

```text
speedup = mean_sequential_time / mean_parallel_time
```

These are the **benchmark settings and calculations implemented in the source**, not verified final performance results. The full 100-million-element benchmark was **not** executed during this documentation review. Two arrays of this size require approximately **800 MB** for integer storage alone, and high cutoffs can create very many threads. Performance results should be recorded on the group's target Linux/WSL machine before claiming the 2× target has been achieved.

Suggested report format for confirmed benchmark results:

| Cutoff | Mean time (s), 3 runs | Relative time (%) | Speedup |
| ---: | ---: | ---: | ---: |
| 0 | [measure] | 100 | 1.00× |
| 1 | [measure] | [calculate] | [calculate] |
| 2 | [measure] | [calculate] | [calculate] |
| 3–10 | [measure or state not tested] | [calculate] | [calculate] |

When measuring performance, run the same input size and seed across configurations and record the CPU, available cores, memory, and whether testing was performed under WSL or native Linux. Timing differences on very small arrays are not a reliable measure of parallel speedup because thread creation and scheduling overhead can dominate the work.

## Known Bugs

1. **Incremental Makefile relinking (reproduced).** The `test-mergesort` link recipe uses `$?`, which includes only prerequisites newer than the target. After a successful build, rebuilding only `test-mergesort.o` can cause a linker error because `mergesort.o` is omitted from the link command. A reliable fix would be to use `$^` to link all prerequisites. A clean build succeeds, but does not eliminate this incremental-build issue.
2. **Unchecked allocation of the two arrays (potential failure).** Both `test-mergesort.c` and `unit-tests.c` allocate `A` and `B` without checking whether `malloc()` returns `NULL`. Large input sizes or insufficient memory could therefore lead to an invalid memory access instead of a controlled error message.
3. **Midpoint arithmetic (edge-case risk).** In the uploaded `mergesort.c`, both recursive implementations still calculate `middle` using `(left + right) / 2`. Adding two large `int` indices may overflow. For valid index ranges, `left + (right - left) / 2` avoids this intermediate addition overflow.
4. **High-cutoff resource limits.** The parallel version creates two threads at each eligible split and exits if thread creation fails. Large cutoffs can cause substantial thread overhead or resource exhaustion, and there is no fallback to sequential sorting after a `pthread_create()` failure. No specific high-cutoff failure is claimed to have occurred in the verified tests.
5. **Debug helper and cleanup limitations.** The optional `printA()` and `printB()` routines always print 100 elements; enabling them with smaller arrays would read out of bounds. In addition, `make clean` does not remove the generated `unit-tests` executable.

These issues do not contradict the passing correctness tests above; several concern unusual inputs, build workflow, or resource limitations rather than the standard sorting logic.


## Reflection and Self Assessment

### Member 1 — Sequential Merge Sort (Ziming)

**Challenge 1 — Managing indices during merging.** The merge function has to move through two ordered ranges without skipping or overwriting values. Keeping the roles of `i`, `j`, and `k` separate, then copying any remaining elements and writing the merged range back to `A`, is central to correctness. **What I learned:** merge sort depends on precise subarray boundaries, and the auxiliary array `B` allows values to be combined without losing elements that are still needed.

**Challenge 2 — Understanding the recursive base case.** A recursive call must stop when `left >= right`; otherwise, splitting could continue indefinitely. The two recursive calls must finish before their parent merges the results. **What I learned:** a divide-and-conquer algorithm needs both a correct stopping condition and a clear guarantee that each subproblem is solved before the combined result is produced.

**Challenge 3 — Reviewing edge cases and arithmetic safety.** Testing only a large random array would miss some boundary behaviours, so the sequential version was also checked with very small and odd-sized inputs. Code review also highlighted the potential overflow risk in the midpoint expression. **What I learned:** passing normal tests is not the same as covering edge cases, and small arithmetic details can matter even when the overall algorithm is correct.

### Member 2 — Parallel Merge Sort (Hang Yu)

**Challenge 1 — Synchronising child threads.** Each recursive split launches threads for the left and right portions. If the parent were to merge before both children finished, it could read data while a child was still writing to it. The implementation therefore joins both threads before merging. **What I learned:** `pthread_join()` provides an ordering point, while keeping threads within separate index ranges helps avoid data races on the shared arrays.

**Challenge 2 — Keeping thread arguments valid.** The thread function requires a pointer argument, so each child receives a separately allocated `struct argument` containing its array bounds and recursion level. The parent frees those argument objects after joining. **What I learned:** memory ownership and object lifetime are important in concurrent code; a thread must not be given a pointer to data that might disappear while it is still running.

**Challenge 3 — Selecting a practical cutoff.** Creating more threads does not always make sorting faster. The code stops spawning threads once it reaches the cutoff and uses sequential recursion thereafter. **What I learned:** parallelism has a cost, so thread creation and scheduling overhead must be balanced against the amount of useful work available in each subarray.

**Challenge 4 – Understanding recursive thread creation.** At first, I found it difficult to understand how parallel_mergesort() could keep creating new threads by calling itself. **What I learned:** I learned that each new thread increases the recursion level, and thread creation stops when the level reaches the cutoff. This helped me understand how recursion and multithreading work together.

### Member 3 — Testing and Performance (Stuart Vass)

**Challenge 1 — Building useful test coverage.** The automated tests include empty and single-element cases, odd and even lengths, and progressively larger arrays. They run the same groups with multiple cutoff values. **What I learned:** boundary cases and different recursion patterns can reveal faults that one normal-sized random test would miss, and a fixed seed makes failures easier to reproduce.

**Challenge 2 — Measuring speedup fairly.** The performance code runs each cutoff three times and compares its average runtime with the sequential baseline. Large arrays reduce the influence of very short timing measurements, but also require significant memory and computational resources. **What I learned:** performance needs repeated measurements under comparable conditions, and the relevant metric is the ratio of parallel time to sequential time rather than an isolated runtime.

**Challenge 3 — Understanding the limits of a passing test.** The current `check_if_sorted()` function checks only nondecreasing order, while the automated test matrix currently uses one random seed. **What I learned:** a good test strategy must verify the intended property carefully; further tests could compare against a trusted reference sort and vary seeds to check that all original elements are preserved.

## Sources Used

- **OpenAI ChatGPT (2026).** Used to discuss sequential merge sort, review code safety and test coverage, assist with Git collaboration. The test claims above were checked against the provided source files and bounded executions.

## Tasks Assignment

### Member 1 – Sequential Merge Sort

- Implement `merge()` and `my_mergesort()`.
- Test sequential merge sort.
- Fix sorting errors and improve the sequential implementation.
- Prepare the README.

### Member 2 – Parallel Merge Sort

- Implement `parallel_mergesort()`.
- Handle thread creation and joining.
- Implement cutoff logic.
- Manage thread arguments and memory.
- Check the README.

### Member 3 – Testing and Performance

- Write automated testing and benchmarking scripts.
- Test different array sizes, cutoff levels, and random seeds.
- Compare sequential and parallel execution times.
- Help optimise the implementation to achieve at least 2x speedup.
- Check the README.

### All Members

- Review the final implementation.
- Fix integration problems.
- Run final tests and check the Gradescope results.
