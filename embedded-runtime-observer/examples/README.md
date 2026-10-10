# Runnable examples

Run `make test` from the module root. All examples use synthetic inputs or independently
written platform models, not live production stacks. See [validation](../docs/validation/stack-validation.md).

| Program | Demonstration |
|---|---|
| `build/basic` | Task=20, Idle=70, ISR=10; 30% CPU load |
| `build/task_isr` | Nested ISR inclusive/exclusive time and CSV Trace |
| `build/multicore` | Independent per-core statistics |
| `build/autosar_concept` | Self-written Task/ISR/Kernel Hook model |
| `build/baremetal` | Mock low-32 timer register and port |
| `build/stack_simulator` | 2048-byte capacity, 640-byte peak, retreat, independent region and synthetic guard change |
| `build/stack_autosar_concept` | Read-only OS callback, shared physical ISR region and unavailable source |

`make stack-test` builds only stack programs/tests, with no time engine or Trace.
`make RTM_BUILD_STACK=0 test` retains the original time-only path.
`make stack-benchmark` measures 20 synthetic workloads at 1/2/8/16 KiB;
`build/benchmark` measures time-engine polling.

The [Stack API guide](../docs/stack-observer.md) explains ownership and unit normalization.
Actual AUTOSAR calls, TASKING builds and TC397 hardware remain platform integration work.
