import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / "tools" / f"{name}.py")
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded


trace = module("trace_to_json")
collector = module("collect_case_evidence")


class HostTools(unittest.TestCase):
    def test_trace_units(self):
        data = trace.convert([dict(core=0, tick=100, sequence=0, kind=1, id=0, value=0)], 1000)
        self.assertEqual(data["traceEvents"][-1]["ts"], 100000)
        self.assertEqual(data["traceEvents"][-1]["ph"], "i")

    def test_trace_loss(self):
        row = dict(core=0, tick=100, sequence=3, kind=1, id=0, value=0)
        with self.assertRaises(ValueError):
            trace.convert([row], 1000)
        self.assertEqual(trace.convert([row], 1000, True)["traceEvents"][0]["name"], "trace_loss")

    def test_bad_frequency(self):
        with self.assertRaises(ValueError):
            trace.convert([], 0)

    def test_collector_preserves_source(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "source"
            src = root / "src" / "hooks.c"
            src.parent.mkdir(parents=True)
            src.write_text("void PreTaskHook(void) {}\n", encoding="utf-8")
            before = src.read_bytes()
            collector.collect(root, base / "evidence.md", ["src"])
            self.assertEqual(src.read_bytes(), before)
            self.assertIn("SHA-256", (base / "evidence.md").read_text())
            with self.assertRaises(ValueError):
                collector.collect(root, root / "evidence.md", ["src"])
            with self.assertRaises(ValueError):
                collector.collect(root, base / "evidence.md", ["src"])

    def test_collector_limits_scope_and_replaces_default_patterns(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "source"
            selected = root / "src" / "hooks.cpp"
            selected.parent.mkdir(parents=True)
            selected.write_text("PreTaskHook();\n" + "// context\n" * 12
                                + "TimingPoint();\nOtherPoint();\n", encoding="utf-8")
            (selected.parent / "notes.txt").write_text("TimingPoint();\n", encoding="utf-8")
            (root / "unselected.c").write_text("TimingPoint();\n", encoding="utf-8")
            output = base / "evidence.md"
            collector.collect(root, output, ["src"], ["(?i)timingpoint", "OtherPoint"])
            report = output.read_text(encoding="utf-8")
            self.assertIn("src/hooks.cpp", report)
            self.assertIn("TimingPoint();", report)
            self.assertIn("OtherPoint();", report)
            self.assertNotIn("PreTaskHook();", report)
            self.assertNotIn("notes.txt", report)
            self.assertNotIn("unselected.c", report)
            self.assertNotIn(str(root), report)

    def test_collector_rejects_target_escape_and_empty_scope(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "source"
            root.mkdir()
            output = base / "evidence.md"
            for targets in ([], ["../outside.c"], [str(base / "outside.c")],
                            [r"C:\source\outside.c"], [r"..\outside.c"]):
                with self.subTest(targets=targets), self.assertRaises(ValueError):
                    collector.collect(root, output, targets)
            self.assertFalse(output.exists())

    def test_collector_skips_external_symlinks(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "source"
            root.mkdir()
            outside = base / "outside.c"
            outside.write_text("PreTaskHook(); /* external marker */\n", encoding="utf-8")
            link = root / "linked.c"
            try:
                link.symlink_to(outside)
            except (OSError, NotImplementedError):
                self.skipTest("symlinks are unavailable")
            output = base / "evidence.md"
            collector.collect(root, output, ["linked.c"])
            report = output.read_text(encoding="utf-8")
            self.assertIn("SKIPPED: outside root", report)
            self.assertNotIn("external marker", report)

    def test_collector_reports_missing_and_oversized_files(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "source"
            root.mkdir()
            (root / "large.c").write_bytes(b"x" * (2 * 1024 * 1024 + 1))
            output = base / "evidence.md"
            collector.collect(root, output, ["missing.c", "large.c"])
            report = output.read_text(encoding="utf-8")
            self.assertIn("MISSING", report)
            self.assertIn("SKIPPED: >2 MiB", report)
            self.assertNotIn("```c", report)

    def test_collector_cli_requires_scope_and_rejects_invalid_pattern(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "source"
            root.mkdir()
            (root / "hooks.c").write_text("PreTaskHook();\n", encoding="utf-8")
            output = base / "evidence.md"
            command = [sys.executable, str(ROOT / "tools" / "collect_case_evidence.py"),
                       str(root), "--output", str(output)]
            missing_scope = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(missing_scope.returncode, 2)
            self.assertIn("--target", missing_scope.stderr)
            bad_pattern = subprocess.run(command + ["--target", "hooks.c", "--pattern", "["],
                                         capture_output=True, text=True)
            self.assertEqual(bad_pattern.returncode, 2)
            self.assertFalse(output.exists())
            valid = subprocess.run(command + ["--target", "hooks.c"],
                                   capture_output=True, text=True)
            self.assertEqual(valid.returncode, 0, valid.stderr)
            self.assertIn("PreTaskHook();", output.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
