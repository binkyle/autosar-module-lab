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
