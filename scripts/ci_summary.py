#!/usr/bin/env python3
"""Render existing reports as GitHub job-summary Markdown, without rerunning work."""
import argparse
import html
import json
import os
from pathlib import Path
from urllib.parse import urlsplit

from coverage_report import summarize_export


def cell(value):
    """Keep report labels from introducing Markdown rows or HTML."""
    return html.escape(str(value), quote=True).replace("|", "&#124;").replace("\n", " ").replace("\r", " ").replace("`", "&#96;")


def artifact_note(name):
    url = os.environ.get("ARTIFACT_URL", "")
    parsed = urlsplit(url)
    if parsed.scheme == "https" and parsed.netloc == "github.com" and not any(c in url for c in "()\n\r"):
        return f"Full reports and raw data: [{name}]({html.escape(url, quote=True)}).\n"
    return f"Full reports and raw data: **{name}** in this run's Artifacts section.\n"


def coverage_cell(metrics):
    count, covered = metrics["count"], metrics["covered"]
    if type(count) is not int or type(covered) is not int or not 0 <= covered <= count:
        raise ValueError("invalid coverage counts")
    return f"{100 * covered / count:.2f}% ({covered:,}/{count:,})" if count else "— (0/0)"


def coverage_summary(directory):
    path = directory / "core.json"
    if not path.is_file():
        return "## Core coverage\n\nCoverage report unavailable; inspect the coverage build/test logs.\n"
    exported = json.loads(path.read_text(encoding="utf-8"))
    totals = summarize_export(exported, [])
    files = [entry for unit in exported["data"] for entry in unit["files"]]
    metrics = ("lines", "branches", "functions", "regions")
    rows = ["## Core coverage", "", "Core sources only; cells show covered/total counts. Zero opportunities are shown as —, not 100%.", "",
            "| Source | Lines | Branches | Functions | Regions |", "| :--- | ---: | ---: | ---: | ---: |"]
    for label, summary in [("Total", totals)] + [(Path(entry["filename"]).name, entry["summary"])
                                                for entry in sorted(files, key=lambda item: item["filename"])]:
        rows.append("| " + cell(label) + " | " + " | ".join(coverage_cell(summary[metric]) for metric in metrics) + " |")
    rows += ["", "The artifact includes per-line/branch HTML (`html/index.html`); GitHub summaries cannot embed that interactive report.",
             "", artifact_note("core-coverage")]
    return "\n".join(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kind", choices=("coverage",))
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()
    report = coverage_summary(args.directory)
    print(report)
    print(f"Job status at summary generation: **{cell(os.environ.get('JOB_STATUS', 'not supplied'))}**.")


if __name__ == "__main__":
    main()
