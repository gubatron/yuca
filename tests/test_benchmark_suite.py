import sys
import argparse
import json
import subprocess
import tempfile
from types import SimpleNamespace
from unittest.mock import patch
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from benchmark_suite import bootstrap_median, compare, invoke, numbers, summarize


def runner_smoke(binary):
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="benchmark-runner-", dir=Path.cwd()) as temporary:
        output = Path(temporary) / "experiment"
        command = [sys.executable, str(root / "scripts" / "benchmark_suite.py"),
                   "--candidate", str(binary), "--baseline", str(binary), "--documents", "24",
                   "--workloads", "search_common", "--top-k", "0,10", "--samples", "3",
                   "--min-sample-ms", "1", "--max-iterations", "10", "--output", str(output)]
        result = subprocess.run(command, capture_output=True, text=True, timeout=60)
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)
        summary = json.loads((output / "summary.json").read_text())
        if len(summary) != 2 or any(entry["statistics"]["candidate"]["checksum"] !=
                                   entry["statistics"]["baseline"]["checksum"] for entry in summary):
            raise RuntimeError("runner did not preserve scenario/fingerprint comparability")


class BenchmarkSuiteTests(unittest.TestCase):
    def rows(self, values, checksum=7):
        return [{"ns_per_operation": value, "checksum": checksum} for value in values]

    def test_reproducible_bootstrap_and_summary(self):
        values = [10, 11, 12, 13, 14]
        self.assertEqual(bootstrap_median(values), bootstrap_median(values))
        result = summarize(self.rows(values))
        self.assertEqual(result["median_ns"], 12)
        self.assertEqual(result["samples"], 5)

    def test_nonfinite_and_empty_timings_are_rejected(self):
        for values in ([], [float("nan")], [float("inf")], [0], [-1]):
            with self.assertRaises(ValueError):
                bootstrap_median(values)

    def test_behavior_changes_block_performance_comparison(self):
        with self.assertRaisesRegex(ValueError, "fingerprints differ"):
            compare(self.rows([10, 11, 12]), self.rows([9, 10, 11], 8))
        with self.assertRaises(ValueError):
            summarize(self.rows([10]) + self.rows([11], 8))

    def test_relative_change_and_scenario_bounds(self):
        self.assertAlmostEqual(compare(self.rows([10] * 3), self.rows([12] * 3))["change_percent"], 20)
        self.assertEqual(numbers("0,10", True), [0, 10])
        with self.assertRaises(argparse.ArgumentTypeError):
            numbers("0")

    def test_native_output_schema_counts_and_metrics_are_verified(self):
        valid = {"schema": 1, "workload": "search_common", "documents": 24,
                 "iterations": 2, "top_k": 10, "operations": 2,
                 "elapsed_ns": 20, "ns_per_operation": 10, "checksum": 7}
        for changed in ({"schema": 2}, {"operations": 200}, {"elapsed_ns": float("nan")},
                        {"ns_per_operation": 99}, {"checksum": "missing"}):
            output = SimpleNamespace(stdout=json.dumps({**valid, **changed}))
            with patch("benchmark_suite.subprocess.run", return_value=output):
                with self.assertRaises(ValueError):
                    invoke(Path("unused"), "search_common", 24, 2, 10)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--binary", type=Path)
    settings, remaining = parser.parse_known_args()
    sys.argv = [sys.argv[0], *remaining]
    if settings.binary:
        runner_smoke(settings.binary.resolve())
    unittest.main()
