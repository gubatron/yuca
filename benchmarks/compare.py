#!/usr/bin/env python3
"""Compare matching Yuca CSV runs; positive changes mean slower (no pass/fail gate)."""
import csv
import statistics
import sys


def load(path):
    with open(path, encoding="utf-8") as source:
        rows = csv.DictReader(line for line in source if not line.startswith("#"))
        result = {}
        for row in rows:
            samples, checksums = result.setdefault(row["workload"], ([], set()))
            samples.append(float(row["ns_per_operation"]))
            checksums.add(row["checksum"])
        return result


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: compare.py before.csv after.csv")
    before, after = map(load, sys.argv[1:])
    if not before or before.keys() != after.keys():
        raise SystemExit("workloads differ or are empty; comparison refused")
    print("workload,before_median_ns,after_median_ns,change_percent,before_range_ns,after_range_ns")
    for name in before:
        left, left_checksums = before[name]
        right, right_checksums = after[name]
        if len(left_checksums) != 1 or left_checksums != right_checksums:
            raise SystemExit(f"{name}: output checksums differ; investigate semantics first")
        old, new = statistics.median(left), statistics.median(right)
        if old <= 0:
            raise SystemExit(f"{name}: invalid baseline")
        print(f"{name},{old:.2f},{new:.2f},{100 * (new / old - 1):+.2f},"
              f"{min(left):.2f}..{max(left):.2f},{min(right):.2f}..{max(right):.2f}")


if __name__ == "__main__":
    main()
