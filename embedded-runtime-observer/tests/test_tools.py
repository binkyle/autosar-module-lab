import importlib.util
from pathlib import Path
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
            src = root / collector.TARGETS[0]
            src.parent.mkdir(parents=True)
            src.write_text("void PreTaskHook(void) {}\n", encoding="utf-8")
            before = src.read_bytes()
            collector.collect(root, base / "evidence.md")
            self.assertEqual(src.read_bytes(), before)
            self.assertIn("SHA-256", (base / "evidence.md").read_text())
            with self.assertRaises(ValueError):
                collector.collect(root, root / "evidence.md")
            with self.assertRaises(ValueError):
                collector.collect(root, base / "evidence.md")


if __name__ == "__main__":
    unittest.main()
