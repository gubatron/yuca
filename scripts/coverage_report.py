#!/usr/bin/env python3
"""Run isolated tests, merge fresh LLVM profiles, and report only Yuca core coverage."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile


def summarize_export(export, required_sources):
    files = [entry for unit in export["data"] for entry in unit["files"]]
    found = {str(Path(entry["filename"]).resolve()) for entry in files}
    missing = {str(Path(path).resolve()) for path in required_sources} - found
    if missing:
        raise ValueError(f"core sources missing from coverage: {sorted(missing)}")
    summary = {}
    for metric in ("lines", "branches", "functions", "regions"):
        count = sum(entry["summary"][metric]["count"] for entry in files)
        covered = sum(entry["summary"][metric]["covered"] for entry in files)
        if covered < 0 or count < covered:
            raise ValueError(f"invalid {metric} coverage counts")
        summary[metric] = {"count": count, "covered": covered,
                           "percent": 100.0 * covered / count if count else None}
    if not summary["lines"]["covered"] or not summary["branches"]["count"]:
        raise ValueError("empty/uninstrumented core coverage cannot pass")
    return summary


def enforce_target_coverage(export):
    """Keep the explicitly qualified files above 90%, without rounding or exclusions."""
    files = [entry for unit in export["data"] for entry in unit["files"]]
    for name in ("key.cpp", "indexer.cpp"):
        matches = [entry for entry in files if Path(entry["filename"]).name == name]
        if len(matches) != 1:
            raise ValueError(f"expected exactly one coverage record for {name}")
        for metric in ("lines", "branches", "functions", "regions"):
            counts = matches[0]["summary"][metric]
            count, covered = counts["count"], counts["covered"]
            if type(count) is not int or type(covered) is not int or not 0 <= covered <= count:
                raise ValueError(f"invalid {name} {metric} counts")
            if count == 0 or covered * 100 <= count * 90:
                raise ValueError(f"{name} {metric} coverage must exceed 90%: {covered}/{count}")


def run(command, **kwargs):
    return subprocess.run([str(part) for part in command], check=True, text=True, **kwargs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--source-dir", type=Path, required=True)
    parser.add_argument("--llvm-cov", required=True)
    parser.add_argument("--llvm-profdata", required=True)
    args = parser.parse_args()
    build, root = args.build_dir.resolve(), args.source_dir.resolve()
    output = build / "coverage"
    output.mkdir(exist_ok=True)
    core = root / "src" / "yuca"
    sources = sorted(core.glob("*.cpp")) + sorted(core.glob("*.hpp")) + sorted(core.glob("*.h"))
    binary = build / "yuca_tests"
    # Each invocation gets a new directory: old profiles can never inflate coverage.
    with tempfile.TemporaryDirectory(prefix="profiles-", dir=output) as directory:
        environment = dict(os.environ, LLVM_PROFILE_FILE=str(Path(directory) / "%m-%p.profraw"))
        run(["ctest", "--test-dir", build, "--output-on-failure"], env=environment)
        profiles = sorted(Path(directory).glob("*.profraw"))
        if not profiles:
            raise RuntimeError("tests produced no LLVM profiles; check coverage instrumentation")
        merged = output / "core.profdata"
        run([args.llvm_profdata, "merge", "-sparse", *profiles, "-o", merged])
    common = [binary, f"-instr-profile={merged}", *sources]
    with (output / "core.txt").open("w", encoding="utf-8") as destination:
        run([args.llvm_cov, "report", *common], stdout=destination)
    exported = json.loads(run([args.llvm_cov, "export", *common], capture_output=True).stdout)
    # CLI source filters must not accidentally admit vendored/system files.
    allowed = {str(path.resolve()) for path in sources}
    for unit in exported["data"]:
        unit["files"] = [entry for entry in unit["files"] if str(Path(entry["filename"]).resolve()) in allowed]
    summary = summarize_export(exported, core.glob("*.cpp"))
    (output / "core.json").write_text(json.dumps(exported, indent=2) + "\n", encoding="utf-8")
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    run([args.llvm_cov, "show", *common, "-format=html", f"-output-dir={output / 'html'}", "-show-branches=count"])
    print((output / "core.txt").read_text(encoding="utf-8"))
    print(f"Core-only summary: {output / 'summary.json'}\nBranch/source HTML: {output / 'html' / 'index.html'}")
    # Retain reports for inspection even when a qualified file regresses.
    enforce_target_coverage(exported)


if __name__ == "__main__":
    main()
