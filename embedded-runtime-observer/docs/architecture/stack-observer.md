# Stack Observer architecture decision

Stack observation is an independent C99 library (`rtm_stack`), alongside the existing
time engine (`rtm`). Neither its headers nor its implementation depend on Trace,
Task/ISR timing state, an RTOS, or a TriCore register. Existing time APIs and public
structures remain unchanged. CMake can build either library separately.

## Ownership and observations

Each caller-owned `rtm_stack_t` represents one physical stack region, with an explicit
core, kind and shared flag. Application mapping tables may map several ISR IDs to the
same object. Aliases preserve shared-stack attribution; no per-ISR peak is inferred.

Application-owned memory uses a bounded byte representation. A caller supplies one
actual contiguous writable C object and its length; low/high reserves and a deepest-end
guard are excluded from effective capacity. Byte accesses support unaligned storage
without typed pointer casts or strict-aliasing violations. Only an offline, safely
owned region may be painted. Observer state and output objects must live outside it.

OS-managed descriptors hold no stack memory pointer. Only a permitted OS callback
produces a sample. An absent callback returns unsupported. The optional OS adapter
normalizes historical used units or minimum free units, checks multiplication overflow,
and reports its effective capacity for comparison with the region. Vendor APIs, IDs,
readiness, core ownership and permitted call contexts remain in the application wrapper.

## Watermark and validity

Linear scanning starts at the deepest usable end. For downward growth this is the low
address end; upward growth is mirrored. All-painted capacity means zero observed peak;
one changed cell at the deepest end means full capacity. A partially changed cell is
counted in full. The first changed cell ends the scan. Arbitrary holes and matching-pattern
writes defeat the monotonic predicate that a binary search would require.

The retained peak is the maximum of observed watermarks in an epoch. Matching-pattern
writes and untouched stack reservations can hide actual depth. `valid` means this sample
satisfied the API/source contract; it does not certify absence of overflow or exact
historical SP depth. Current usage remains explicitly unsupported. Guard checks describe
boundary evidence, with damage returning `E_GUARD` and invalid diagnostic data; guards
are never automatically repaired. Repainting an offline application region starts a new
epoch. OS source resets require a coordinated application rebind and source contract.

## Access and scheduling

All object accesses, measurements, copies and application-buffer writes are serialized
by the caller. Scanning requires quiescent source memory: an inactive stack or a platform
protocol that excludes every writer throughout the scan. `volatile` byte reads do not
replace synchronization. Mutex simulations exercise this contract; production ISR/Hook
paths should not run size-dependent scans or blocking remote OS queries.

`rtm_stack_measure` acquires a new observation; `rtm_stack_snapshot` copies the last result
without scanning or calling the OS. A low-priority report task can rotate through regions
and publish copies through IOC/messages. A remote backend must preserve its own progress
requirements; disabling interrupts around a synchronous XSignal query can deadlock.

## Growth and costs

With capacity C, cell size w and unchanged cells U, a payload scan inspects U cells when
all paint is intact, or U+1 cells when it encounters a changed cell. Worst case is C/w;
paint is O(C+guard), guard checking O(guard), cached copy O(1) in region size. A one-cell
guard has constant cost. No scan shortcut or allocation is introduced in v0.2.0.

CSA remains a separate future observer: it would need owner-core capture, bounded chain
validation, address/access checks, and an explicit consistency protocol for FCX/LCX/PCXI.
Its block counts cannot be merged with ordinary stack bytes. No CSA traversal is shipped.

See [API and usage](../stack-observer.md), [TC397 integration](../cases/tc397-autosar-stack-study.md)
and [validation](../validation/stack-validation.md).
