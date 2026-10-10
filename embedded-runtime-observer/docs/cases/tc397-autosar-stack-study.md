# TC397 / AUTOSAR stack observation

This integration guide abstracts the supplied engineering investigation into an independent
observer contract. No Vector implementation or business configuration is included. The Windows
TC397 project is unavailable in this environment; the report has not been revalidated against
its generated source, TASKING build or running ECU.

## Configuration and source contract

The investigation describes Vector MICROSAR stack painting with 0xAAAAAAAA, 4-byte cells,
downward growth, Task/ISR usage queries and cross-core XSignal. In that reported configuration,
usage is historical **used bytes**, rather than current SP usage or minimum free words.
Treat `Os_GetTaskStackUsage` and `Os_GetISRStackUsage` as supplier-extension integration points,
not portable AUTOSAR services guaranteed across suppliers or releases. Use the actual generated
headers and the installed OS manual for prototypes, enablement and call-level permissions.

| Integration item | Establish in the target project |
|---|---|
| DaVinci configuration | Measurement enablement, stack sharing, generated object IDs and initialization |
| Effective capacity | Usable OS interval excluding actual reserves/sentinels; same interval as the usage API |
| Task mapping | Task ID to physical stack region; several Tasks may share a region |
| ISR mapping | ISR ID to owning core and physical interrupt stack; publish shared attribution |
| Source lifetime | OS initialization complete; source reset and observer generation coordinated |
| Call level | Legal Task/report context, interrupts and OS-application permissions |
| Remote query | XSignal availability, destination progress and measured blocking time |

Do not infer TC397 gap sizes from a TC4xx implementation or equate an allocated/linker length
with usable capacity. If the exact OS denominator cannot be established, leave the measurement
unavailable instead of publishing an apparently precise percentage.

## Adapter boundary

Keep the generated IDs and actual OS call inside an application `rtm_stack_os_read_fn`.
For a verified byte-peak source, configure `PEAK_USED` with unit size 1. Configure one
OS-managed observer per physical region, with no memory pointer. A source failure produces an
invalid diagnostic result; lack of a legal API produces unsupported, not a private-memory scan.
The [standalone mock](../../examples/stack_autosar_concept.c) exercises this path without a vendor SDK.

A shared core ISR stack should be queried and reported as one region. Map its users to that
object and mark `shared=1`; a peak cannot be assigned to one ISR merely because its ID was used
for the query. For shared task stacks, apply the same physical-region rule.

## Periodic acquisition and publication

A CDD/report task can iterate over a bounded subset of regions each activation, normalize results,
update observer objects and publish copies. Select the interval from actual query cost and desired
freshness; an investigation's reporting cadence is not a general sampling requirement.

Call local and remote APIs only in documented contexts. Cross-core calls may synchronously wait
for an OS service. Keep progress-critical interrupts available and measure the maximum observed
wait on the target. Do not perform scans, prints or potentially blocking queries in ISR/Hook code.
Object serialization must protect cached statistics without violating the OS backend's protocol.

For target observation, WinIDEA can inspect an authorized inactive/offline stack interval and
compare the paint boundary with the OS-returned peak. Record the effective bounds, growth direction,
word size, observed frame history and debugger halt behavior. A debugger screenshot alone does not
verify all SP reservations or matching-pattern writes. No target screenshots are claimed here.

## Guard, protection and CSA

Watermark describes observed historical depth. A boundary sentinel detects changes when inspected;
it does not prevent memory corruption. MPU protection can trap accesses covered by its configured
regions and permissions; actual task-stack coverage and trap handling must be established from the
project's generated protection setup. Stack sharing or API names do not prove MPU coverage.

TriCore CSA is a separate context resource. Architectural context areas occupy 64-byte blocks;
FCX identifies the free-context chain, LCX participates in depletion handling, and PCXI identifies
the saved context chain. The Infineon architecture manual describes FCD and FCU conditions; these
are context-management events, not ordinary C-stack byte percentages.

| Future CSA observation | Required boundary |
|---|---|
| Capacity/free/low-water counts | Units are context blocks; ordinary stack bytes remain separate |
| Register capture | Owner-core, coherent read protocol and permitted context |
| Chain traversal | Bounded nodes, valid-address checks, link validation and access permission |
| Sampling | Consistency while hardware allocates/releases contexts; separately budgeted |
| Failure handling | Independent validity and depletion diagnostics |

No CSA register access, chain traversal or trap injection is shipped. Implement a separate observer
only after proving a consistent target protocol and validating it against hardware evidence.

## Verification status

| Level | Current evidence |
|---|---|
| Host Tested | Portable library, all growth/cell configurations, errors, guards, unit conversion and concurrency |
| AUTOSAR Concept Verified | Self-written runnable OS callback model, shared ISR aliases, readiness failure |
| TC397 Build Verified | **Not yet verified**; no TASKING/MICROSAR build environment |
| TC397 Hardware Verified | **Not yet verified**; no ECU timing, WinIDEA or controlled workload run |

The next target acceptance run should verify generated API signatures and bounds, compile with
TASKING, compare OS peaks under representative load, measure remote-query/report costs and verify
shared-region publication. Use ordinary permitted workloads, not destructive stack-overflow tests.

## References

- [Infineon TriCore TC1.6.2 architecture, Volume 1](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-architecture-vol1-usermanual-en.pdf?fileId=5546d46276fb756a01771bc4c2e33bdd)
- [Infineon: free-context depletion](https://community.infineon.com/t5/Knowledge-Base-Articles/What-happens-when-there-is-not-enough-free-context-AURIX-MCU/ta-p/341078)
- [Vector MICROSAR runtime analysis webinar](https://cdn.vector.com/cms/content/events/2019/Webinars19/Vector_Webinar_Efficient_Run-Time_Analysis_of_MICROSAR_projects_with_OS_and_RTE_tracing.pdf)
- [FreeRTOS high-water-mark API](https://freertos.org/uxTaskGetStackHighWaterMark.html)
- [Observer API and units](../stack-observer.md)
