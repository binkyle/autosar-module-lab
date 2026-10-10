# Port checklist and minimal integration

The engine is timer/OS-independent. `port.h` separates a clock callback, saved interrupt mask,
and a per-core object. See the runnable `examples/autosar_concept.c` and `examples/baremetal.c`.

| Environment | Event source | Serialization and reporting | Status |
|---|---|---|---|
| Host mock | Explicit synthetic ticks/events | Single-thread driver or per-core pthread mutex | Tested |
| Bare metal | Main-loop active/idle transitions and IRQ wrappers | Architecture mask saved/restored; owner-core polling | Host mock tested; target unverified |
| AUTOSAR | Verified Task/ISR/ARTI boundaries with ID lookup | Only legal, documented hook operations; copied snapshot via IOC | Concept only; Experimental |
| TC3xx STM | BSP-supplied TIM0 low-32 pointer | Local observation serialization; bounded polling gap | Register strategy compiles in mock; hardware Unverified |

The caller obtains/maps a Core ID and selects a port object before calling the wrapper. No core-ID
lookup or interrupt API is imposed on the generic core. Do not assume physical hardware IDs are
contiguous logical OS IDs. Allocate and initialize every core on that core; do not share open point
stacks or migrate tasks between observer objects.

Make the periodic poll gap shorter than both the configured gap and timer half-range. Check every
returned status without calling logging/printing code inside the critical section. A safe fault
response can set a diagnostic flag for the reporting task and start a new epoch at a known context.
Measure critical-section length on the target. The C99 engine's absence of internal locks is not a
claim that its public APIs are reentrant on one object.

## Stack acquisition port

Stack Observer does not use the time-event wrapper or its interrupt mask. Application-owned
offline memory may use the portable byte scanner; an OS-managed object has no memory pointer
and uses only a permitted OS callback. No legal API means unsupported, not an internal-stack scan.

`stack_os.h` isolates reusable normalization from vendor headers. Its application `read`
callback checks OS readiness, object IDs, owner core, call level and access permission.
Historical used-byte peaks use PEAK_USED/unit=1. Minimum free words use MIN_FREE and
the actual `sizeof(StackType_t)`; multiplication and range errors are rejected.
Both the provider and observer must use the same effective capacity.

Several ISR/Task IDs may map to one physical region. Query that region once and mark it shared;
do not present its peak as a private measurement for one user. The
[runnable OS mock](../examples/stack_autosar_concept.c) models two ISR aliases without importing
Vector source. Actual MICROSAR usage services are supplier-dependent integration points.

Serialize observer accesses and publish copies, while preserving the backend's progress rules.
Potentially blocking XSignal calls must run in documented report-task contexts with required
interrupts available. Do not apply a generic interrupt-disabled time-engine wrapper to them.
For application memory, the caller must exclude all buffer writers throughout a scan.

See [API and scheduling](../docs/stack-observer.md), [TC397 stack study](../docs/cases/tc397-autosar-stack-study.md)
and [executed Host validation](../docs/validation/stack-validation.md). Target build, hardware,
MPU and CSA verification remain separate work.
