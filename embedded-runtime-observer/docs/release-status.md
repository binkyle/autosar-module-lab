# v0.1.0 module distribution

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

The frozen v0.1.0 archive remains available through the
[Runtime Measurement series](https://binkyle.github.io/blog/Autosar/RuntimeMeasurement/).
Its core was verified in the blog repository's
[GCC/Clang and Trace 0/64 workflow](https://github.com/binkyle/binkyle.github.io/actions/runs/37914920526).
Updated module distribution notes do not change the core implementation.

## CI layout

`.github/workflows/ci.yml` inside this module is a standalone workflow template.
GitHub Actions reads workflows only from the repository root, so this nested template does
not enable automatic CI in autosar-module-lab. The repository's independent-build convention
is preserved. Local verification commands and the already executed archive CI are documented
in [validation.md](validation.md).

## Platform status

Portable C99 engine and Host models: verified.
TC397 / MICROSAR / TASKING adapter: Experimental / Unverified.
Business source and hardware have not been accessed or changed by this publication.

