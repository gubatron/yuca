"""Regression tests for visible CI summaries; no GitHub/network access required."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from ci_summary import artifact_note, benchmark_summary, coverage_summary


class CoverageSummaryTests(unittest.TestCase):
    def fixture(self, directory):
        metrics = {metric: {"count": 4, "covered": 3} for metric in ("lines", "branches", "functions", "regions")}
        export = {"data": [{"files": [{"filename": "/project/src/yuca/indexer.cpp", "summary": metrics}]}]}
        (directory / "core.json").write_text(json.dumps(export), encoding="utf-8")
        return export

    def test_total_and_source_rows_use_counts_not_stored_percentages(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.fixture(directory)
            report = coverage_summary(directory)
            self.assertIn("| Total | 75.00% (3/4)", report)
            self.assertIn("| indexer.cpp | 75.00% (3/4)", report)
            self.assertIn("core-coverage", report)
            self.assertNotIn("/project/", report)

    def test_missing_report_is_not_a_zero_or_success_claim(self):
        with tempfile.TemporaryDirectory() as temporary:
            report = coverage_summary(Path(temporary))
            self.assertIn("unavailable", report)
            self.assertNotIn("0.00%", report)

    def test_zero_branches_and_hostile_labels(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            export = self.fixture(directory)
            entry = export["data"][0]["files"][0]
            entry["filename"] = "/project/<script>|`\n.cpp"
            entry["summary"]["functions"] = {"count": 0, "covered": 0}
            (directory / "core.json").write_text(json.dumps(export))
            report = coverage_summary(directory)
            self.assertIn("— (0/0)", report)
            self.assertIn("&lt;script&gt;&#124;&#96; .cpp", report)
            self.assertNotIn("<script>", report)

    def test_invalid_counts_are_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            export = self.fixture(directory)
            export["data"][0]["files"][0]["summary"]["lines"]["covered"] = 5
            (directory / "core.json").write_text(json.dumps(export))
            with self.assertRaises(ValueError):
                coverage_summary(directory)

    def test_cli_writes_renderable_markdown_and_failure_status(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.fixture(directory)
            result = subprocess.run([sys.executable, str(ROOT / "scripts/ci_summary.py"), "coverage", str(directory)],
                                    check=True, capture_output=True, text=True,
                                    env={**os.environ, "JOB_STATUS": "failure"})
            self.assertTrue(result.stdout.startswith("## Core coverage\n"))
            self.assertIn("**failure**", result.stdout)

    def test_only_safe_artifact_links_are_rendered(self):
        with patch.dict(os.environ, {"ARTIFACT_URL": "https://github.com/gubatron/yuca/actions/runs/1/artifacts/2"}):
            self.assertIn("[core-coverage](https://github.com/", artifact_note("core-coverage"))
        with patch.dict(os.environ, {"ARTIFACT_URL": "javascript:alert(1)"}):
            self.assertNotIn("javascript", artifact_note("core-coverage"))


class BenchmarkSummaryTests(unittest.TestCase):
    def fixture(self, directory):
        (directory / "benchmarks.csv").write_text(
            "# compiler=test documents=1000 samples=3\nworkload,sample,ns_per_operation,checksum\n"
            "index_document,0,1000,7\nindex_document,1,3000,7\nindex_document,2,2000,7\n", encoding="utf-8")
        (directory / "environment.txt").write_text("commit123\ncompiler <test>\n", encoding="utf-8")
        (directory / "scaling").mkdir()
        scenarios = [{"workload": "search_common", "documents": 1000, "top_k": 0, "calibration_capped": True,
                      "statistics": {"candidate": {"samples": 3, "median_ns": 2000,
                                                   "median_bootstrap_95pct_ns": [1000, 3000]}}}]
        (directory / "scaling/summary.json").write_text(json.dumps(scenarios), encoding="utf-8")
        record = {"workload": "search_top10", "documents": 1000, "iterations": 100,
                  "new_calls": 200, "new_requested_bytes": 3000, "peak_extra_live_new_bytes": 100,
                  "live_new_bytes_before": 500, "live_new_bytes_after": 500, "peak_process_rss_bytes": 1048576}
        (directory / "allocations.jsonl").write_text(json.dumps(record) + "\n", encoding="utf-8")
        return scenarios

    def test_all_sections_have_units_statistics_and_caveats(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.fixture(directory)
            report = benchmark_summary(directory)
            self.assertIn("| index_document | document | 3 | 2.000 | 1.000–3.000 |", report)
            self.assertIn("| search_common | candidate | 1,000 | 0 | query | 3 | 2.000 | 1.000–3.000 | yes |", report)
            self.assertIn("| search_top10 | 1,000 | 100 | 200 | 3,000 | 100 | +0 | 1.00 |", report)
            self.assertIn("compiler &lt;test&gt;", report)
            self.assertIn("process-lifetime peak", report)
            self.assertIn("not a speedup claim", report)
            self.assertIn("release-benchmarks", report)

    def test_missing_and_partial_reports_do_not_claim_success(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.assertEqual(benchmark_summary(directory).count("unavailable"), 3)
            (directory / "benchmarks.csv").write_text("workload,sample,ns_per_operation,checksum\nsearch_top10,0,1000,7\n")
            report = benchmark_summary(directory)
            self.assertEqual(report.count("unavailable"), 2)
            self.assertIn("| search_top10 | query | 1 | 1.000", report)

    def test_nonfinite_and_invalid_intervals_fail_instead_of_being_published(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            scenarios = self.fixture(directory)
            scenarios[0]["statistics"]["candidate"]["median_ns"] = float("nan")
            (directory / "scaling/summary.json").write_text(json.dumps(scenarios))
            with self.assertRaises(ValueError):
                benchmark_summary(directory)
            scenarios[0]["statistics"]["candidate"]["median_ns"] = 4000
            (directory / "scaling/summary.json").write_text(json.dumps(scenarios))
            with self.assertRaises(ValueError):
                benchmark_summary(directory)

    def test_zero_rss_is_unavailable_not_zero_memory_usage(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.fixture(directory)
            path = directory / "allocations.jsonl"
            record = json.loads(path.read_text())
            record["peak_process_rss_bytes"] = 0
            path.write_text(json.dumps(record))
            self.assertIn("| +0 | unavailable |", benchmark_summary(directory))

    def test_cli_and_workflow_publish_to_job_summary(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.fixture(directory)
            result = subprocess.run([sys.executable, str(ROOT / "scripts/ci_summary.py"), "benchmarks", str(directory)],
                                    check=True, capture_output=True, text=True)
            self.assertIn("## Release benchmarks", result.stdout)
        workflow = (ROOT / ".github/workflows/native.yml").read_text()
        self.assertIn('ci_summary.py benchmarks build/ci >> "$GITHUB_STEP_SUMMARY"', workflow)
        self.assertIn('ci_summary.py coverage build/coverage/coverage >> "$GITHUB_STEP_SUMMARY"', workflow)


if __name__ == "__main__":
    unittest.main()
