#!/usr/bin/env python3
"""Collect local C/C++ excerpts from explicitly selected files or directories.

Output must be outside the source root. Generated evidence retains source paths,
symbols and excerpts; keep it private and review locally before any publication.
Project-specific paths and patterns are supplied by the caller, not bundled here.
"""
import argparse
import hashlib
import re
from pathlib import Path, PurePosixPath, PureWindowsPath

DEFAULT_PATTERN = (
    r"\b(?:PreTaskHook|PostTaskHook|Rtm_Start|Rtm_Stop|GetCoreID|Idle)\b"
)
SOURCE_SUFFIXES = {".c", ".h", ".cc", ".cpp", ".hpp"}


def selected_paths(root, targets):
    """Expand only caller-selected root-relative paths, without following escapes."""
    if not targets:
        raise ValueError("at least one root-relative target is required")
    paths = set()
    for target in targets:
        name = str(target).replace("\\", "/")
        relative = PurePosixPath(name)
        if (not name or relative.is_absolute() or PureWindowsPath(name).drive
                or ".." in relative.parts):
            raise ValueError("targets must be relative paths without parent traversal")
        path = root.joinpath(*relative.parts)
        resolved = path.resolve()
        if resolved != root and root not in resolved.parents:
            paths.add(path)  # Report the skipped target, without reading it.
        elif path.is_dir():
            paths.update(p for p in path.rglob("*")
                         if p.suffix.lower() in SOURCE_SUFFIXES and p.is_file())
        else:
            paths.add(path)
    return paths


def collect(root, output, targets, patterns=None):
    root, output = root.resolve(), output.resolve()
    if output == root or root in output.parents:
        raise ValueError("output must be outside source root; source tree is read-only")
    if not root.is_dir():
        raise ValueError("source root does not exist")
    if output.exists():
        raise ValueError("output already exists; choose a new evidence filename")
    paths = selected_paths(root, targets)
    matchers = [re.compile(value) for value in (patterns or [DEFAULT_PATTERN])]
    text = ["# Local runtime measurement evidence\n",
            "Private source evidence, not a publication-ready case.\n",
            "## Exact-file excerpts and related call sites\n"]
    for path in sorted(paths):
        name = path.relative_to(root).as_posix()
        # A symlink outside the root is not part of this collection.
        if root not in path.resolve().parents:
            text.append(f"### {name}\nSKIPPED: outside root\n")
            continue
        if not path.is_file():
            text.append(f"### {name}\nMISSING\n")
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
            if any(pattern.search(line) for pattern in matchers):
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
    with output.open("x", encoding="utf-8") as stream:
        stream.write("\n".join(text))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--target", action="append", required=True,
                        help="root-relative file or directory; repeat to select more")
    parser.add_argument("--pattern", action="append",
                        help="regular expression; repeat to match any supplied pattern")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        collect(args.root, args.output, args.target, args.pattern)
    except (ValueError, OSError, re.error) as error:
        parser.error(str(error))
    print(f"Read-only evidence written to {args.output}")


if __name__ == "__main__":
    main()
