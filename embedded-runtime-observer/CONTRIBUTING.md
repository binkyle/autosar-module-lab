# Contributing

Build with `make test`; use `python3 tools/check_format.py` for baseline formatting.
CI additionally builds with GCC/Clang, Trace=0/64, address/undefined-behavior sanitizers, GCC analyzer,
and cppcheck. `.clang-format` is an optional authoring style; the required dependency-free format
gate checks whitespace/final newlines, not a complete C formatter.

For a timing change, state the executing owner and add an independently computed event example or
invariant. Preserve no-allocation/nonblocking behavior. Document any new ISR, counter, synchronization,
or clock assumption. Keep all capacity definitions consistent across compilation units.

Ports must state the hardware/OS/compiler version, hook coverage, clock frequency/continuity,
Core-ID mapping, masking protocol, and whether target build/hardware checks were performed. Do not
mark a mock port production-ready. Do not submit vendor/internal source or generated configuration.

Use focused commits for architecture, implementation, verification, and documentation. An issue or
test showing an actual integration need is preferred before adding transports or format dependencies.
