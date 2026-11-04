import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from coverage_report import summarize_export


class CoverageReportTests(unittest.TestCase):
    def fixture(self, covered=3, count=4):
        return {"data": [{"files": [{"filename": str(Path("core.cpp").resolve()),
            "summary": {metric: {"count": count, "covered": covered}
                        for metric in ("lines", "branches", "functions", "regions")}}]}]}

    def test_computes_real_core_counts(self):
        self.assertEqual(summarize_export(self.fixture(), ["core.cpp"])["branches"]["percent"], 75)

    def test_missing_core_source_fails(self):
        with self.assertRaisesRegex(ValueError, "missing"):
            summarize_export(self.fixture(), ["not-linked.cpp"])

    def test_empty_or_uninstrumented_report_fails(self):
        with self.assertRaisesRegex(ValueError, "uninstrumented"):
            summarize_export(self.fixture(0, 0), ["core.cpp"])

    def test_invalid_coverage_counts_fail(self):
        with self.assertRaisesRegex(ValueError, "invalid"):
            summarize_export(self.fixture(5, 4), ["core.cpp"])


if __name__ == "__main__":
    unittest.main()
