# Host verification record — 2026-10-09

Environment: Linux x86-64, GCC 13.3.0, Python 3.12. Core/examples compile as C99 with
`-Wall -Wextra -Wpedantic -Werror`. This record is about Host behavior, not MCU timing.

| Check | Result |
|---|---|
| Named C contracts | 37 scenarios pass |
| Random conservation oracle | 100,000 transitions, independent tick counts match |
| Concurrent snapshots | 2 writers + 2 readers on 2 cores, 200,000 serialized operations pass |
| Python Host tools | 4 unittest cases pass |
| Examples | basic, task_isr, multicore, autosar_concept, baremetal pass |
| Trace disabled | Same C suite passes with capacity 0 |
| Reduced static capacity | Tasks/ISRs/Points=8, depth=4, Trace=0 passes |
| GCC `-fanalyzer` | Core/port compile with no diagnostics |
| Address + undefined-behavior sanitizer | 37 scenarios pass with leak detection disabled |
| ThreadSanitizer | Concurrent test passes; no race diagnostics |
| Baseline format | whitespace/final-newline check passes |
| TC397 / MICROSAR / TASKING | Not available; no target build or hardware test |

LeakSanitizer's process/thread inspection is blocked by the local execution environment. The
ASan/UBSan rerun used `ASAN_OPTIONS=detect_leaks=0`; it is not a passed leak check. The core has no
heap API. The hosted CI configuration uses normal sanitizer defaults.

Local CMake, Clang, and cppcheck binaries were unavailable. Make/GCC performed actual local builds;
the published source archive subsequently passed the executed
[GCC/Clang × Trace 0/64 workflow](https://github.com/binkyle/binkyle.github.io/actions/runs/37914920526).
Each matrix job passed 8 CTest entries with ASan/UBSan; a fifth job passed cppcheck 2.13.0 and GCC -fanalyzer.
This is the frozen time-release record. v0.2.0 adds active repository-root CI and
records full time regression plus new stack validation in
[stack-validation.md](validation/stack-validation.md). The nested module workflow remains
a standalone template.

## Reproduction

```sh
make test
gcc -std=c99 -Wall -Wextra -Werror -fanalyzer -Iinclude -c src/rtm.c src/port.c
gcc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Iinclude tests/test_rtm.c src/rtm.c src/port.c -o build/contracts-asan
ASAN_OPTIONS=detect_leaks=0 ./build/contracts-asan
gcc -std=c99 -O1 -g -fsanitize=thread -Iinclude tests/test_concurrent.c \
    src/rtm.c src/port.c -pthread -o build/concurrent-tsan
./build/concurrent-tsan
python3 tools/check_format.py
./build/task_isr > build/trace.csv
python3 tools/trace_to_json.py build/trace.csv build/trace.json --frequency 1000
```

The `--frequency 1000` setting describes the **mock example's ticks** as milliseconds; it is not the
real STM frequency. Raw normal-suite output: [host-tests.txt](validation/host-tests.txt).

## Benchmark

Compile with `-O2`. `benchmark.c` samples 200,000 calls using CLOCK_MONOTONIC; a second loop measures
the timer-pair baseline. Values include timer overhead, scheduler interference, and measurement
noise. There is no target interrupt masking in this benchmark, and polling does not emit trace.

| Workload | Raw mean | Timer-pair mean | Maximum observed |
|---|---:|---:|---:|
| Task poll, default configuration | 27.93 ns | 22.95 ns | 57,797 ns |
| Poll, 8 nested ISRs + 8 active points | 39.38 ns | 24.70 ns | 148,333 ns |

These are one run's **raw** data, not a stable average across machines or a worst-case bound.
Do not subtract the baseline and treat that estimate as WCET. Hook, switch, snapshot, and timer
port costs need separate target measurements. Raw output: [host-benchmark.txt](validation/host-benchmark.txt).

Default layout: core=11,144 bytes, snapshot=3,648 bytes; reduced layout: core=3,504,
snapshot=1,920 bytes. These are Host `sizeof` values and exclude application/output storage.
