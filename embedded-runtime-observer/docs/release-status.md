# v0.2.0 module distribution

The canonical source location is
[autosar-module-lab/embedded-runtime-observer](https://github.com/binkyle/autosar-module-lab/tree/main/embedded-runtime-observer).

The module is stored as a self-contained source directory beside CommPolicyCoordinator.
A normal repository clone includes both modules; each is built independently.

## Reproduce

```sh
git clone https://github.com/binkyle/autosar-module-lab.git
cd autosar-module-lab/embedded-runtime-observer
make test
```

Core/example/test implementation originates from the locally verified source commit
`c0da6377c286f88800fc16657cb75dd0bf470007`. The monorepo import records these files in its
own history. The original standalone staging history is retained in the earlier delivery bundle.

v0.2.0 adds the independent Stack Observer, OS-unit adapter, two Host examples, 39 stack
contract scenarios, concurrent-copy simulation and a 20-workload benchmark. The time-engine
source and public time APIs remain unchanged. See the [Stack API](stack-observer.md),
[documentation index](README.md) and [executed validation](validation/stack-validation.md).

The frozen v0.1.0 archive remains available through the
[Runtime Measurement series](https://binkyle.github.io/blog/Autosar/RuntimeMeasurement/).
Its core was verified in the blog repository's
[GCC/Clang and Trace 0/64 workflow](https://github.com/binkyle/binkyle.github.io/actions/runs/37914920526).
This frozen archive contains the original time release and does not contain Stack Observer.
Clone the canonical repository for v0.2.0; no separate observer repository is needed.

## CI layout

`.github/workflows/ci.yml` inside this module is a standalone workflow template.
GitHub Actions reads workflows only from the repository root. The active
[Embedded Runtime Observer workflow](https://github.com/binkyle/autosar-module-lab/actions/workflows/embedded-runtime-observer.yml)
therefore lives at the repository root and runs only for module/workflow changes. It checks
GCC/Clang × Trace 0/64 × Stack ON/OFF, Stack-only and library-only builds, sanitizers, static
analysis, Python tools and a benchmark artifact. Each repository module still builds independently.

The nested template is intended for a standalone copy whose module directory is the repository
root. It is not itself an active monorepo workflow. Historical time-release validation remains
in [validation.md](validation.md); v0.2.0 regression and costs are in
[stack-validation.md](validation/stack-validation.md).

## Platform status

Portable C99 time engine, Stack Observer and Host models: verified.
AUTOSAR stack query/shared-region model: concept verified using independently written mock APIs.
TC397 / MICROSAR / TASKING adapter: Experimental / Unverified.
No business source was accessed or changed; the supplied stack investigation informs the
integration guide but is not a new target build or hardware verification.

