import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from coverage_report import enforce_target_coverage, summarize_export


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

    def targeted_fixture(self):
        return {"data": [{"files": [{"filename": str(Path(name).resolve()),
            "summary": {metric: {"count": 11, "covered": 10}
                        for metric in ("lines", "branches", "functions", "regions")}}
            for name in ("key.cpp", "indexer.cpp")]}]}

    def test_targeted_files_pass_only_above_ninety_percent(self):
        enforce_target_coverage(self.targeted_fixture())
        for file_index in (0, 1):
            for metric in ("lines", "branches", "functions", "regions"):
                for covered, count in ((9, 10), (8, 10), (0, 0), (9, 11)):
                    export = self.targeted_fixture()
                    export["data"][0]["files"][file_index]["summary"][metric] = {"count": count, "covered": covered}
                    with self.assertRaisesRegex(ValueError, "must exceed 90%"):
                        enforce_target_coverage(export)

    def test_gate_rejects_missing_duplicate_and_invalid_records(self):
        export = self.targeted_fixture()
        export["data"][0]["files"].pop()
        with self.assertRaisesRegex(ValueError, "exactly one"):
            enforce_target_coverage(export)
        export = self.targeted_fixture()
        export["data"][0]["files"].append(export["data"][0]["files"][0])
        with self.assertRaisesRegex(ValueError, "exactly one"):
            enforce_target_coverage(export)
        export = self.targeted_fixture()
        export["data"][0]["files"][0]["summary"]["lines"] = {"count": 10, "covered": 11}
        with self.assertRaisesRegex(ValueError, "invalid"):
            enforce_target_coverage(export)


if __name__ == "__main__":
    unittest.main()
