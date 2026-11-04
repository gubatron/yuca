#!/usr/bin/env python3
"""Calibrated, interleaved native benchmark samples with bootstrap uncertainty."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import platform
import random
import statistics
import subprocess
import sys

WORKLOADS = ("index", "replace", "remove", "clear", "copy", "search_rare", "search_common",
             "search_grouped", "search_distance", "search_empty")


def bootstrap_median(values, seed=444, repetitions=2000):
    if not values or any(not math.isfinite(value) or value <= 0 for value in values):
        raise ValueError("timings must be positive finite values")
    rng = random.Random(seed)
    estimates = sorted(statistics.median(rng.choices(values, k=len(values))) for _ in range(repetitions))
    return [estimates[int((repetitions - 1) * fraction)] for fraction in (0.025, 0.975)]


def summarize(rows):
    if not rows or len({row["checksum"] for row in rows}) != 1:
        raise ValueError("missing samples or inconsistent output fingerprints")
    values = [row["ns_per_operation"] for row in rows]
    interval = bootstrap_median(values)
    return {"samples": len(values), "median_ns": statistics.median(values),
            "min_ns": min(values), "max_ns": max(values),
            "stdev_ns": statistics.stdev(values) if len(values) > 1 else None,
            "median_bootstrap_95pct_ns": interval, "checksum": rows[0]["checksum"]}


def compare(left, right):
    if not left or not right:
        raise ValueError("missing comparison samples")
    if {row["checksum"] for row in left + right} != {left[0]["checksum"]}:
        raise ValueError("baseline/candidate fingerprints differ; investigate behavior before speed")
    a = [row["ns_per_operation"] for row in left]
    b = [row["ns_per_operation"] for row in right]
    rng = random.Random(444)
    estimates = sorted(100 * (statistics.median(rng.choices(b, k=len(b))) /
                              statistics.median(rng.choices(a, k=len(a))) - 1) for _ in range(2000))
    return {"change_percent": 100 * (statistics.median(b) / statistics.median(a) - 1),
            "change_bootstrap_95pct_percent": [estimates[49], estimates[1949]]}


def invoke(binary, workload, documents, iterations, top_k):
    command = [str(binary), workload, str(documents), str(iterations), str(top_k)]
    result = subprocess.run(command, check=True, capture_output=True, text=True, timeout=180)
    row = json.loads(result.stdout)
    if any(row.get(key) != value for key, value in (("schema", 1), ("workload", workload),
           ("documents", documents), ("iterations", iterations), ("top_k", top_k))):
        raise ValueError("benchmark schema/scenario mismatch")
    expected_operations = iterations * (documents if workload in ("index", "replace", "remove") else 1)
    if row.get("operations") != expected_operations:
        raise ValueError("incorrect benchmark operation count")
    for key in ("operations", "elapsed_ns", "ns_per_operation"):
        if not isinstance(row.get(key), (int, float)) or not math.isfinite(row[key]) or row[key] <= 0:
            raise ValueError(f"invalid benchmark metric: {key}")
    if not math.isclose(row["elapsed_ns"] / row["operations"], row["ns_per_operation"], rel_tol=1e-9):
        raise ValueError("inconsistent timing denominator")
    if type(row.get("checksum")) is not int or row["checksum"] < 0:
        raise ValueError("missing output fingerprint")
    return row


def numbers(text, allow_zero=False):
    values = [int(part) for part in text.split(",")]
    if not values or any(value < (0 if allow_zero else 1) or value > 100000 for value in values):
        raise argparse.ArgumentTypeError("scenario values must fit supported range 1..100000 (top-k allows zero)")
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--documents", default="100,1000,10000")
    parser.add_argument("--top-k", default="0,10")
    parser.add_argument("--workloads", default=",".join(WORKLOADS))
    parser.add_argument("--samples", type=int, default=9)
    parser.add_argument("--min-sample-ms", type=float, default=50)
    parser.add_argument("--max-iterations", type=int, default=10000)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not 3 <= args.samples <= 100 or not 1 <= args.max_iterations <= 100000 or not 0 < args.min_sample_ms <= 10000:
        parser.error("samples=3..100, iterations=1..100000, and sample target=(0,10000] ms required")
    try:
        documents, limits = numbers(args.documents), numbers(args.top_k, True)
    except (ValueError, argparse.ArgumentTypeError) as error:
        parser.error(str(error))
    workloads = args.workloads.split(",")
    if any(workload not in WORKLOADS for workload in workloads):
        parser.error("unknown workload")
    binaries = {"candidate": args.candidate.resolve()}
    if args.baseline:
        binaries["baseline"] = args.baseline.resolve()
    args.output.mkdir(parents=True, exist_ok=False) # Never overwrite an earlier experiment.
    metadata = {"platform": platform.platform(), "machine": platform.machine(),
                "python": sys.version, "command": sys.argv, "working_directory": str(Path.cwd()),
                "binaries": {name: {"path": str(path), "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
                             for name, path in binaries.items()},
                "notes": "Fresh process per sample with one internal untimed warmup. Setup, validation and teardown excluded. Record compiler/flags/power settings separately."}
    (args.output / "environment.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    report = []
    rng = random.Random(444)
    with (args.output / "raw.csv").open("x", newline="", encoding="utf-8") as destination:
        fields = ["configuration", "sample", "schema", "workload", "documents", "iterations", "top_k",
                  "operations", "elapsed_ns", "ns_per_operation", "checksum"]
        writer = csv.DictWriter(destination, fieldnames=fields)
        writer.writeheader()
        for workload in workloads:
            for count in documents:
                for limit in limits if workload.startswith("search_") else [0]:
                    pilot = invoke(binaries["candidate"], workload, count, 1, limit)
                    desired = max(1, math.ceil(args.min_sample_ms * 1e6 / pilot["elapsed_ns"]))
                    iterations = min(args.max_iterations, desired)
                    for binary in binaries.values():
                        invoke(binary, workload, count, iterations, limit) # untimed warmup process
                    rows = {name: [] for name in binaries}
                    for sample in range(args.samples):
                        order = list(binaries)
                        rng.shuffle(order) # Interleave configurations; avoid always timing one first.
                        for name in order:
                            row = invoke(binaries[name], workload, count, iterations, limit)
                            rows[name].append(row)
                            writer.writerow({"configuration": name, "sample": sample, **row})
                            destination.flush()
                    entry = {"workload": workload, "documents": count, "top_k": limit,
                             "iterations": iterations, "calibration_capped": desired > args.max_iterations,
                             "statistics": {name: summarize(data) for name, data in rows.items()}}
                    if "baseline" in rows:
                        entry["comparison"] = compare(rows["baseline"], rows["candidate"])
                    report.append(entry)
                    print(json.dumps(entry), flush=True)
    (args.output / "summary.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
