# Stack Observer validation — 2026-10-10

Local environment: Linux x86-64, GCC 13.3.0, C99, Python 3.12. Strict compilation uses
`-Wall -Wextra -Wpedantic -Werror`. The new module leaves time-core source and APIs unchanged.
Host results demonstrate the portable contracts, not real ECU stack depth or target WCET.

## Executed tests

| Check | Result |
|---|---|
| Stack named contracts | 39/39 pass |
| Independent byte oracle | 8,000 randomized operations across growth/cell configurations pass |
| Stack concurrency | 2 region writers + 2 readers; 200,000 mutex-serialized operations pass |
| Stack examples | Simulator and independent AUTOSAR callback mock pass |
| Existing time regression | 37/37 scenarios, including 100,000 oracle transitions, pass |
| Existing concurrency/examples/tools | 200,000 serialized operations, 5 examples and 4 Python tests pass |
| ASan + UBSan | Complete time and stack suites/examples pass |
| GCC static analysis | Stack core and normalization adapter pass `-fanalyzer` |
| Optional builds | Make stack-only and Stack-disabled full regression pass |
| Freestanding compilation | Stack core and OS adapter compile separately with `-ffreestanding` |

Coverage includes all-painted/partial/full usage, retained peaks and repaint epochs, 1/2/8/16 KiB,
DOWN/UP, 1/2/4/8-byte cells, custom byte patterns, unaligned memory, partial cells, reserves,
numeric address wrap, invalid/null/zero/range configurations, guard/no-guard, warning thresholds,
snapshot caching, shared-region aliases, source failures, provider capacity drift and unit overflow.
Pattern-collision and nonmonotonic-hole tests explicitly exercise watermark's limitations.

The mutex test checks the caller's serialization contract; it does not establish race freedom for
unsynchronized target stack reads. TC397/MICROSAR/TASKING, OS hook legality, XSignal timing, MPU,
actual SP depth and CSA are not verified by these tests.

LeakSanitizer cannot inspect process tasks in the local ptraced environment. The completed
ASan/UBSan run used `ASAN_OPTIONS=detect_leaks=0`; no passed leak check is claimed. Hosted
CI retains normal sanitizer settings. Local Clang, CMake and cppcheck are unavailable, so
their actual results must come from the active GitHub workflow rather than a local claim.

Raw records: [normal suite](stack-host-tests.txt), [ASan/UBSan](stack-sanitize-tests.txt),
[optional builds](stack-optional-builds.txt), [benchmark](stack-host-benchmark.txt).

## Benchmark

`make stack-benchmark` builds at `-O2`. Each of 20 workloads receives 200 warmups and
10,000 timed calls using CLOCK_MONOTONIC, 4-byte cells and a 4-byte guard. Raw times include
timer calls, scanning, guard checks, statistics/copy work and scheduler noise.
The timer-pair mean in this recorded run was 24.05 ns.

| Capacity | Peak | Scanned cells | Raw mean (ns) | Maximum observed (ns) |
|---:|---:|---:|---:|---:|
| 1024 | 0 | 256 | 635.81 | 91508 |
| 1024 | 256 | 193 | 532.04 | 132911 |
| 1024 | 512 | 129 | 350.68 | 39039 |
| 1024 | 920 | 27 | 113.68 | 70166 |
| 1024 | 1024 | 1 | 37.05 | 13871 |
| 2048 | 0 | 512 | 1416.48 | 302655 |
| 2048 | 512 | 385 | 928.96 | 52439 |
| 2048 | 1024 | 257 | 720.88 | 69665 |
| 2048 | 1840 | 53 | 160.59 | 42144 |
| 2048 | 2048 | 1 | 37.07 | 41 |
| 8192 | 0 | 2048 | 5425.19 | 969510 |
| 8192 | 2048 | 1537 | 3963.51 | 752874 |
| 8192 | 4096 | 1025 | 2607.45 | 223827 |
| 8192 | 7372 | 206 | 571.44 | 100091 |
| 8192 | 8192 | 1 | 38.84 | 18017 |
| 16384 | 0 | 4096 | 10128.73 | 248514 |
| 16384 | 4096 | 3073 | 7616.84 | 292160 |
| 16384 | 8192 | 2049 | 5119.26 | 281474 |
| 16384 | 14744 | 411 | 1007.71 | 46751 |
| 16384 | 16384 | 1 | 37.43 | 12459 |

90% depths are rounded down to a cell boundary. An all-painted region is the maximum
scan-work case; a changed deepest cell exits after one cell. Average cost grows with the
unchanged prefix length. The large observed maxima demonstrate Host noise; they are not
WCET bounds and should not be transferred to TC397. The longest mean here is about
10.13 µs for 16 KiB; measure real backend cost and scheduling interference on the target
before choosing a report-task budget.

## Active CI and reproduction

The repository-root
[`embedded-runtime-observer.yml`](../../../.github/workflows/embedded-runtime-observer.yml)
runs GCC/Clang × Trace 0/64 × Stack ON/OFF with ASan/UBSan, an independent Stack-only
CMake build, a library-only build, cppcheck/GCC analysis, Python tools and an uploaded Host
benchmark. Stack ON has 12 CTest entries, Stack OFF 8, and Stack-only 4. Workflow execution
and its per-job result are visible in
[GitHub Actions](https://github.com/binkyle/autosar-module-lab/actions/workflows/embedded-runtime-observer.yml).
The nested module workflow is only a standalone template.

```sh
make test
make BUILD_DIR=build-stack-only stack-test
make BUILD_DIR=build-no-stack RTM_BUILD_STACK=0 test
gcc -std=c99 -Wall -Wextra -Werror -fanalyzer -Iinclude -c src/stack.c ports/stack_os.c
make BUILD_DIR=build-sanitize CFLAGS="-O1 -g -std=c99 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer" test
```

Use the LeakSanitizer override only in an environment with its documented process-inspection
restriction. Target integration acceptance is described in the [TC397 study](../cases/tc397-autosar-stack-study.md).
