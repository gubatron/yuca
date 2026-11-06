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
from ci_summary import artifact_note, coverage_summary


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


if __name__ == "__main__":
    unittest.main()
