#!/usr/bin/env python3
"""Dependency-free baseline format checks; not a clang-format substitute."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
errors = []
for folder in ("include", "src", "ports", "examples", "tests", "tools"):
    for path in sorted((root / folder).rglob("*")):
        if not path.is_file() or path.suffix not in {".c", ".h", ".py"}:
            continue
        content = path.read_bytes()
        if not content.endswith(b"\n"):
            errors.append(f"{path.relative_to(root)}: missing final newline")
        for number, line in enumerate(content.splitlines(), 1):
            if line != line.rstrip():
                errors.append(f"{path.relative_to(root)}:{number}: trailing whitespace")
            if b"\t" in line:
                errors.append(f"{path.relative_to(root)}:{number}: tab outside Makefile")
print("\n".join(errors) if errors else "PASS source whitespace and final newlines")
sys.exit(bool(errors))
