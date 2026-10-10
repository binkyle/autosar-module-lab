# Stack Observer API and usage

Stack Observer is the optional `rtm_stack` C99 library. It observes ordinary stack-memory
watermarks, independently of the time engine, Trace, an RTOS or MCU registers. One
caller-owned `rtm_stack_t` represents one physical region. IDs are metadata, not a registry.

## Build and start

```sh
make stack-test
./build/stack_simulator
./build/stack_autosar_concept
make stack-benchmark
cmake -S . -B build-stack -DRTM_BUILD_RUNTIME=OFF
cmake --build build-stack --parallel
ctest --test-dir build-stack --output-on-failure
```

For a target library without Host tests/examples, add
`-DBUILD_TESTING=OFF -DRTM_BUILD_EXAMPLES=OFF`. Alternatively compile `src/stack.c`
and, when OS normalization is needed, `ports/stack_os.c`, with `include/` on the path.
Only standard C support is needed; no allocation, lock or transport is imposed.

The [complete simulator](../examples/stack_simulator.c) prints synthetic values:

```text
Simulated Stack Capacity : 2048 B
Peak Usage : 640 B
Remaining : 1408 B
Usage : 31.25%
```

It paints a 2052-byte array: 4 bytes of guard plus 2048 bytes of effective capacity.
It is not measuring the Host process's executing stack.

## Four operations

| Operation | Behavior |
|---|---|
| `rtm_stack_init(stack, region)` | Validate and copy the descriptor; never touch source memory; start epoch 1 |
| `rtm_stack_paint(stack)` | Application-only offline initialization; fill payload and the configured guard; exclude reserves |
| `rtm_stack_measure(stack, out)` | Acquire a new bounded scan or an authorized OS sample, then copy the result |
| `rtm_stack_snapshot(stack, out)` | Copy the previous result and return its status, without scanning/querying |

Repainting an already painted, offline application region clears the peak and increments
the epoch. Epoch overflow is rejected before writes. Reinitialization rebinds the object and
starts epoch 1; persist a separate application generation if identities must span rebinds.
OS source resets require coordination and reinitialization rather than silent peak clearing.

Check every returned status and the result's `measurement_status`, `valid` and guard state.
A failed init leaves an unusable zeroed object. A null object/output or uninitialized measurement
returns `E_STATE` without writing output. Before a paint or successful OS sample, a cached
result is unavailable (`E_STATE`).

## Application-owned memory

`owner=RTM_STACK_APPLICATION` accepts a real writable contiguous C object, a byte length,
low/high reserve sizes, guard size, direction and cell pattern. Zero defaults select downward
growth and Task metadata. The caller proves the object bounds and access rights; the library
can reject numeric address wrap but cannot discover the true size of a raw pointer.

```text
effective capacity = region_bytes - reserved_low_bytes
                     - reserved_high_bytes - guard_bytes
```

Capacity must be nonzero; cell size is 1, 2, 4 or 8 bytes. Capacity and guard must be cell
multiples. Reserve sizes need not be multiples. Pattern arrays encode bytes in increasing
address order, independent of CPU endianness. The first `cell_bytes` bytes repeat for each
cell; a 4-byte array of 0xaa encodes the familiar 0xAAAAAAAA pattern.

| Growth | Payload offset | Guard offset | Scan order |
|---|---|---|---|
| DOWN | low reserve + guard | low reserve | Lowest payload cell toward higher addresses |
| UP | low reserve | low reserve + capacity | Highest payload cell toward lower addresses |

Byte access permits unaligned memory without typed word loads, alignment exceptions or aliasing
violations. Platform stack-ABI alignment is still the integrator's responsibility. Painting
initializes this observer's own guard; existing platform sentinels should instead be reserved
and checked by their owner. No reserved byte is painted or scanned.

Paint only memory that is offline and contains no live data. Never paint the currently executing
stack. Observer, config, adapter and output objects must live outside the measured object.
Scanning requires quiescent memory throughout the operation, such as an inactive stack with
an established owner protocol. This library does not stop tasks or suspend other cores.

## Values, validity and watermark limits

| Result | Meaning |
|---|---|
| `capacity_bytes` | Effective denominator after reserves/guard, or OS-authorized effective capacity |
| `peak_used_bytes` | Maximum of observed watermark peaks within this epoch |
| `remaining_bytes` | Capacity minus retained peak, not current SP free space |
| `usage_percent` | 100 × peak / capacity, computed without integer multiplication overflow |
| `current_used_bytes/current_status` | 0 / `E_UNSUPPORTED`; no current SP backend is shipped |
| `guard` | NONE, INTACT, DAMAGED or UNAVAILABLE; separate from watermark |
| `warning` | Remaining <= configured threshold; threshold 0 disables it |
| `scanned_cells` | Payload cells inspected in the last measurement, including the first changed cell; 0 for OS queries |
| `region_id/core_id/kind/shared` | Physical-region identity and attribution metadata |

The scan stops at the first cell differing from paint. An intact payload reports zero peak;
a changed cell at the deepest end reports full capacity. A partially changed cell counts in
full. With U intact cells and cell size w, it inspects U+1 cells when a change is found, or
all C/w cells when every cell is intact. Worst-case scan cost is O(C); guard cost is O(guard).
There is no binary search: arbitrary paint holes do not give a monotonic predicate.

Returning from a stack frame usually does not repaint memory. The observer also retains a
previously observed higher watermark even if the buffer later resembles paint again. Writes
that equal the pattern, or SP reservations that never write, can still hide true historical
depth before observation. `valid=1` means the source/API contract succeeded, not that peak
is exact SP depth or that an overflow never happened.

Guard damage returns `E_GUARD`, `valid=0`, and bounded diagnostic values. The library does
not repair the guard or latch a permanent fault; each check describes current evidence.
Applications may latch their own diagnostic. An unavailable guard can accompany a valid OS
watermark. Query/range/provider failures retain previous numeric diagnostics but mark them
invalid and guard unavailable. Never publish these as a fresh successful measurement.

## OS-managed regions and units

`owner=RTM_STACK_OS_MANAGED` requires all memory, region, reserve, guard and cell-size fields
to be zero. Supply `os_capacity_bytes`, `query` and `user`. There is no direct scan fallback
and no OS-memory painting. With no callback, measurement returns `E_UNSUPPORTED`.

A query returns `rtm_stack_provider_sample_t` with historical peak **used bytes**, effective
capacity and guard evidence. Capacity must match the descriptor, and peak must not exceed it.
Provider capacity checks prevent a changed denominator being accepted silently.

The optional `rtm_stack_os_adapter_t` provides a reusable unit conversion:

| Source contract | semantics | bytes_per_unit | Conversion |
|---|---|---|---|
| Documented peak used bytes | PEAK_USED | 1 | peak = raw |
| Documented peak used words | PEAK_USED | actual word size | peak = raw × unit |
| FreeRTOS minimum free words | MIN_FREE | `sizeof(StackType_t)` | peak = effective capacity − raw × unit |

Integer overflow and samples beyond capacity are rejected. FreeRTOS's public high-water mark
means the minimum free stack since task creation and is expressed in words; preserve that
semantic explicitly ([official API](https://freertos.org/uxTaskGetStackHighWaterMark.html)).
The adapter imports no vendor header; the application `read` callback performs the actual API call
and validates readiness, IDs, access permissions, core and legal context.

Several Task/ISR IDs may alias one physical region. Set `shared=1`, query once per region and
publish the region identity. The [AUTOSAR mock](../examples/stack_autosar_concept.c) maps two ISR
IDs to the same object. The result is not one ISR's private peak.

## Access and scheduling contract

Serialize **every** object access, including init, paint, measure and snapshot, together with
all application-buffer writes. Different independent regions may run in parallel. Volatile byte
reads only express observations; they do not provide atomicity, memory ordering or race freedom.
The concurrency test uses one mutex per physical region and checks copies under the same lock.

Place size-dependent scans and possibly blocking remote OS queries in permitted report tasks.
Do not wrap an XSignal query in a time-engine interrupt mask: its backend may require those
interrupts to make progress. A backend's OS synchronization and the observer's object lock have
different responsibilities. Publish copied results through an application-defined IOC/message
protocol. Sampling budgets and target timings must be verified in the actual platform.

## Platform scope

Host C99, mocks and byte-level boundaries are tested. MICROSAR extensions, TASKING builds,
TC397 hardware, MPU coverage and WinIDEA comparisons are not target-verified here.
CSA is a separate resource and has no implementation in v0.2.0.
See [architecture](architecture/stack-observer.md), [TC397 study](cases/tc397-autosar-stack-study.md)
and [validation](validation/stack-validation.md).
