#!/usr/bin/env python3
"""Read-only local investigation helper. Output must be outside the source root.

The generated evidence contains proprietary source excerpts: review locally;
do not commit it to a public repository. This script itself contains no vendor
implementation or internal absolute path.
"""
import argparse
import hashlib
import re
from pathlib import Path

PATTERN = re.compile(
    r"ISR_PREEMPTED|TASK_RECORD|PreTaskHook|PostTaskHook|Rtm_Start|Rtm_Stop|"
    r"MISC_OS_.*Isr|Idle|STM[0-9]|IfxStm|CORE_ID|CoreId|GetCoreID|"
    r"TaskSwitch|TaskResume|Os_TaskWait|Os_TaskIdle|60000|60[uU]?\b"
)
TARGETS = [
    "CDD/Common/public/Misc/CPULoadMeasure.c", "CDD/Common/public/Misc/misc.h",
    "Davinci/Applications/Appl/Source/Os_Callout_Stubs.c",
    "Davinci/Applications/Appl/GenData/Os_Task_Lcfg.c",
    "Davinci/Applications/Appl/GenData/Os_Scheduler_Lcfg.c",
    "Davinci/Applications/Appl/GenData/Os_Counter_Lcfg.c",
    "Davinci/Components/Os/Implementation/Os_Task.c",
    "Davinci/Components/Os/Implementation/Os_Scheduler.c",
]


def collect(root, output):
    root, output = root.resolve(), output.resolve()
    if output == root or root in output.parents:
        raise ValueError("output must be outside source root; source tree is read-only")
    if not root.is_dir():
        raise ValueError("source root does not exist")
    if output.exists():
        raise ValueError("output already exists; choose a new evidence filename")
    paths = set(root / path for path in TARGETS)
    for directory in [root / "CDD/Common/public/Misc", root / "Davinci/Applications/Appl/Source"]:
        if directory.is_dir():
            paths.update(p for p in directory.rglob("*") if p.suffix in {".c", ".h"})
    text = ["# Local runtime measurement evidence\n",
            "Private source evidence, not a publication-ready case.\n",
            "## Exact-file excerpts and related call sites\n"]
    for path in sorted(paths):
        name = path.relative_to(root).as_posix()
        if not path.is_file():
            text.append(f"### {name}\nMISSING\n")
            continue
        # A symlink outside the root is not part of this collection.
        if root not in path.resolve().parents:
            text.append(f"### {name}\nSKIPPED: outside root\n")
            continue
        raw = path.read_bytes()
        if len(raw) > 2 * 1024 * 1024:
            text.append(f"### {name}\nSKIPPED: >2 MiB\n")
            continue
        try:
            decoded = raw.decode("utf-8")
        except UnicodeDecodeError:
            decoded = raw.decode("gb18030", errors="replace")
        lines = decoded.splitlines()
        selected = set()
        for i, line in enumerate(lines):
            if PATTERN.search(line):
                selected.update(range(max(0, i-4), min(len(lines), i+5)))
        text.append(f"### {name}\nSHA-256: `{hashlib.sha256(raw).hexdigest()}`\n")
        text.append("```c")
        for i in sorted(selected):
            text.append(f"{i+1:6d}: {lines[i]}")
        text.append("```\n")
    text.append("## Next manual checks\n\n"
                "Follow each RTM caller into generated measurement tables; inspect inactive #if branches, "
                "all ISR categories, Idle routing, timer frequency, physical-to-logical Core ID mapping, "
                "and same-core synchronization. The excerpt list is not a complete call graph.\n")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(text), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    collect(args.root, args.output)
    print(f"Read-only evidence written to {args.output}")


if __name__ == "__main__":
    main()
