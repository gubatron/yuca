"""Guard the workflow action/runtime policy without a YAML dependency.

actionlint validates YAML/expressions separately; these checks cover the reviewed
action declarations and runner labels that previously emitted hosted notices.
"""
from pathlib import Path
import re
import sys
import unittest

WORKFLOW = Path(__file__).resolve().parents[1] / ".github/workflows/native.yml"
if len(sys.argv) == 2:
    WORKFLOW = Path(sys.argv.pop())


class NativeWorkflowTests(unittest.TestCase):
    def test_checkout_is_pinned_to_reviewed_node24_release(self):
        declarations = re.findall(r"uses: actions/checkout@([^\n]+)", WORKFLOW.read_text())
        self.assertTrue(declarations)
        for declaration in declarations:
            self.assertEqual(declaration, "3d3c42e5aac5ba805825da76410c181273ba90b1 # v7.0.1")

    def test_artifact_upload_is_pinned_to_reviewed_node24_release(self):
        declarations = re.findall(r"uses: actions/upload-artifact@([^\n]+)", WORKFLOW.read_text())
        self.assertTrue(declarations)
        for declaration in declarations:
            self.assertEqual(declaration, "cf430e030ddbb5b0abf93d22962f4752f3646cd9 # v7.0.2")


if __name__ == "__main__":
    unittest.main()
