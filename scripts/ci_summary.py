#!/usr/bin/env python3
"""Render existing reports as GitHub job-summary Markdown, without rerunning work."""
import argparse
import csv
import html
import json
import math
import os
from pathlib import Path
import statistics
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


def number(value, minimum=0):
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise ValueError("invalid benchmark metric")
    return result


def operation_unit(workload):
    return "document" if workload in ("index", "replace", "remove", "index_document", "remove_document") else (
        "index" if workload in ("clear", "copy") else "query")


def benchmark_summary(directory):
    rows = ["## Release benchmarks", "",
            "Informational hosted-runner measurements, not a speedup claim or a timing gate. Small sample counts do not establish tail latency.", ""]
    environment = directory / "environment.txt"
    if environment.is_file():
        rows += ["<details><summary>Commit, runner and compiler</summary>", "",
                 "<pre>" + html.escape(environment.read_text(encoding="utf-8")[:8000]) + "</pre>", "</details>", ""]
    classic = directory / "benchmarks.csv"
    rows += ["### Microbenchmarks", ""]
    if classic.is_file():
        groups = {}
        with classic.open(encoding="utf-8") as source:
            for row in csv.DictReader(line for line in source if not line.startswith("#")):
                groups.setdefault(row["workload"], []).append(number(row["ns_per_operation"], minimum=1e-300))
        if not groups:
            raise ValueError("empty benchmark CSV")
        rows += ["| Workload | Unit | Samples | Median (µs/op) | Min–max (µs/op) |",
                 "| :--- | :--- | ---: | ---: | ---: |"]
        for workload, values in sorted(groups.items()):
            rows.append(f"| {cell(workload)} | {operation_unit(workload)} | {len(values)} | "
                        f"{statistics.median(values) / 1000:,.3f} | {min(values) / 1000:,.3f}–{max(values) / 1000:,.3f} |")
    else:
        rows.append("Microbenchmark report unavailable; inspect the benchmark step logs.")
    snapshots = directory / "snapshots.csv"
    if snapshots.is_file():
        groups = {}
        with snapshots.open(encoding="utf-8") as source:
            for entry in csv.DictReader(source):
                key = (int(entry["documents"]), int(entry["bytes"]))
                if min(key) <= 0:
                    raise ValueError("invalid snapshot benchmark cardinality")
                groups.setdefault(key, []).append(entry)
        if not groups:
            raise ValueError("empty snapshot benchmark CSV")
        rows += ["", "### .yuca checkpoint timings", "",
                 "Medians in milliseconds. Save includes encoding and file sync; load includes read, validation and bulk reconstruction. Zero file timings mean not measured. Warm filesystem cache; no power-loss durability or latency guarantee.", "",
                 "| Documents | Bytes | Samples | Encode (ms) | Bulk decode (ms) | Replay decode (ms) | Save (ms) | Load (ms) |",
                 "| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |"]
        for (documents, byte_count), entries in sorted(groups.items()):
            timings = []
            for metric in ("encode_ms", "bulk_decode_ms", "replay_decode_ms", "save_ms", "load_ms"):
                value = statistics.median(number(entry[metric]) for entry in entries)
                timings.append(f"{value:,.3f}" if value else "not measured")
            rows.append(f"| {documents:,} | {byte_count:,} | {len(entries)} | " + " | ".join(timings) + " |")
    rows += ["", "### Scaling suite", ""]
    scaling = directory / "scaling" / "summary.json"
    if scaling.is_file():
        scenarios = json.loads(scaling.read_text(encoding="utf-8"))
        if not scenarios:
            raise ValueError("empty scaling report")
        rows += ["Top-k 0 means unlimited. Capped means calibration hit the iteration limit; the target duration may not have been reached.", "",
                 "| Workload | Build | Documents | Top-k | Unit | Samples | Median (µs/op) | Median bootstrap 95% interval (µs/op) | Capped |",
                 "| :--- | :--- | ---: | ---: | :--- | ---: | ---: | ---: | :--- |"]
        for scenario in scenarios:
            for configuration, metrics in sorted(scenario["statistics"].items()):
                median = number(metrics["median_ns"], minimum=1e-300) / 1000
                low, high = [number(value) / 1000 for value in metrics["median_bootstrap_95pct_ns"]]
                if not low <= median <= high:
                    raise ValueError("invalid benchmark interval")
                samples = number(metrics["samples"], minimum=1)
                documents = number(scenario["documents"], minimum=1)
                limit = number(scenario["top_k"])
                rows.append(f"| {cell(scenario['workload'])} | {cell(configuration)} | {documents:,.0f} | {limit:,.0f} | "
                            f"{operation_unit(scenario['workload'])} | {samples:,.0f} | {median:,.3f} | {low:,.3f}–{high:,.3f} | "
                            f"{'yes' if scenario['calibration_capped'] else 'no'} |")
    else:
        rows.append("Scaling report unavailable or incomplete; inspect the benchmark step logs.")
    rows += ["", "### Allocation and memory diagnostics", "",
             "Allocation figures are totals across the measured window, not per-operation values. RSS is the process-lifetime peak (including setup); diagnostic allocator overhead is included. These are not CPU timing measurements.", ""]
    allocations = directory / "allocations.jsonl"
    if allocations.is_file():
        records = [json.loads(line) for line in allocations.read_text(encoding="utf-8").splitlines() if line.strip()]
        if not records:
            raise ValueError("empty allocation report")
        rows += ["| Workload | Documents | Iterations | new calls | Requested bytes | Extra peak live bytes | Net live bytes | Peak RSS (MiB) |",
                 "| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |"]
        for record in records:
            values = [number(record[key]) for key in ("documents", "iterations", "new_calls", "new_requested_bytes", "peak_extra_live_new_bytes")]
            delta = number(record["live_new_bytes_after"]) - number(record["live_new_bytes_before"])
            rss = number(record["peak_process_rss_bytes"])
            rows.append("| " + cell(record["workload"]) + " | " + " | ".join(f"{value:,.0f}" for value in values) +
                        f" | {delta:+,.0f} | " + (f"{rss / 1048576:,.2f}" if rss else "unavailable") + " |")
    else:
        rows.append("Allocation report unavailable; inspect the benchmark step logs.")
    rows += ["", artifact_note("release-benchmarks")]
    return "\n".join(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kind", choices=("coverage", "benchmarks"))
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()
    report = coverage_summary(args.directory) if args.kind == "coverage" else benchmark_summary(args.directory)
    print(report)
    print(f"Job status at summary generation: **{cell(os.environ.get('JOB_STATUS', 'not supplied'))}**.")


if __name__ == "__main__":
    main()
