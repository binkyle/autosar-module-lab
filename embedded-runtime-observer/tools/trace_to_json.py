#!/usr/bin/env python3
"""Export CSV events as Perfetto/Chrome instant events, not reconstructed spans.

Each core has a separate process and a local epoch. This export does not prove
clock synchronization, reconstruct CPU slices, or implement CTF/BTF.
"""
import argparse
import csv
import json
from pathlib import Path

KINDS = ["task_switch", "isr_enter", "isr_exit", "point_begin", "point_end", "user"]


def convert(rows, frequency, allow_loss=False):
    if frequency <= 0:
        raise ValueError("frequency must be positive")
    result, previous, clocks, cores = [], {}, {}, set()
    for row in rows:
        core, tick, sequence, kind, ident, value = (
            int(row[k]) for k in ("core", "tick", "sequence", "kind", "id", "value")
        )
        if min(core, tick, sequence, kind, ident, value) < 0 or kind >= len(KINDS):
            raise ValueError("invalid event")
        if core in previous and (tick < clocks[core] or sequence <= previous[core]):
            raise ValueError("events must be monotonically ordered per core")
        expected = previous.get(core, -1) + 1
        if sequence != expected:
            if not allow_loss:
                raise ValueError("trace loss detected; use --allow-loss to retain partial events")
            result.append({"ph": "i", "s": "g", "name": "trace_loss", "pid": core,
                           "tid": 0, "ts": tick * 1000000 // frequency,
                           "args": {"expected_sequence": expected, "actual_sequence": sequence}})
        if core not in cores:
            cores.add(core)
            result.append({"ph": "M", "name": "process_name", "pid": core, "tid": 0,
                           "args": {"name": f"Core {core} (independent clock epoch)"}})
        result.append({"ph": "i", "s": "t", "name": KINDS[kind], "pid": core,
                       "tid": 0, "ts": tick * 1000000 // frequency,
                       "args": {"id": ident, "value": value, "sequence": sequence}})
        previous[core], clocks[core] = sequence, tick
    return {"traceEvents": result, "displayTimeUnit": "ms",
            "otherData": {"clock_semantics": "Independent per-core ticks since rtm_init; no alignment",
                          "intervals": "Instant events only; function points measure CPU time"}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--frequency", required=True, type=int, help="ticks per second")
    parser.add_argument("--allow-loss", action="store_true")
    args = parser.parse_args()
    with args.csv.open(encoding="utf-8", newline="") as stream:
        data = convert(csv.DictReader(stream), args.frequency, args.allow_loss)
    args.output.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
