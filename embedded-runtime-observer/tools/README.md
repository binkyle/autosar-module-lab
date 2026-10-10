# Local evidence collection

`collect_case_evidence.py` reads only the files or directories selected with
`--target`. The public tool includes no project-specific directory layout,
filenames or private matching symbols. Supply those choices on your own machine.

```sh
python3 tools/collect_case_evidence.py /path/to/source \
  --target src --target generated \
  --output /path/to/local-evidence/runtime.case-evidence.md
```

Targets are relative to the source root. Directories select `.c`, `.h`, `.cc`,
`.cpp` and `.hpp` files recursively. Explicit files may use any extension. An
absolute target or `..` is rejected; symlinks outside the root are skipped.
Missing files and files over 2 MiB are recorded without excerpts.

The default regular expression matches the generic names `PreTaskHook`,
`PostTaskHook`, `Rtm_Start`, `Rtm_Stop`, `GetCoreID` and `Idle`. Repeated
`--pattern` options replace the default and match any supplied expression:

```sh
python3 tools/collect_case_evidence.py /path/to/source \
  --target src/hooks.c --pattern 'PreTaskHook' --pattern 'PostTaskHook' \
  --output /path/to/local-evidence/hooks.case-evidence.md
```

Output must be outside the source root and must not already exist. The source
tree is read-only. UTF-8 is tried first, then GB18030 with replacement. Reports
contain relative file paths, SHA-256 values, line numbers and source excerpts
with up to four lines of context on each side. They remain private evidence:
the collector does not anonymize proprietary source, symbols or filenames.
Keep reports and private command configurations outside public repositories.
The module ignores `local-evidence/` and `*.case-evidence.md` as an additional
guard against accidentally committing local reports.
