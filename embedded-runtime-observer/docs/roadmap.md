# Resource observation roadmap

v0.2.0 keeps the existing time engine and adds an independently selectable Stack Observer.
These are capability groups, not a reason to introduce separate registries or subsystems.

| Capability | Implemented behavior | Next engineering boundary |
|---|---|---|
| CPU Observer | Per-core Idle/Task/ISR/Kernel windows and load | Target coverage and observation budget |
| Task Observer | Dispatch and same-owner function timing | Actual OS/runnable boundaries |
| ISR Observer | Nesting, inclusive/exclusive timing | Cat1/Cat2 and real wrapper coverage |
| Stack Observer | Effective capacity, DOWN/UP byte watermark, retained peak, warnings and guard | Generated OS bounds, permitted query contexts and physical sharing |
| Trace | Optional bounded event FIFO and Host export | Consumer-driven formats/transport and clock alignment |
| Platform adapters | Clock/serialization port, Host mocks, OS-unit normalization | TASKING/MICROSAR/TC397 target build and hardware evidence |

The next release should prioritize a real OS adapter with documented API signatures, correct
effective denominators, shared-region mapping, bounded reporting and measured XSignal behavior.
Target-owned copies and a tested IOC/message protocol are preferable to reading live remote state.
Current SP usage needs a separate trustworthy execution-context backend before it can be exposed.

CSA observation is a future independent module with context-block units, owner-core coherent
capture, bounded chain validation and its own validity. It must not inflate ordinary stack capacity.
Algorithm shortcuts should only be introduced with a proven invariant, meaningful benchmark gains
and an unchanged ownership/synchronization contract. The v0.2.0 linear scan remains the default.
