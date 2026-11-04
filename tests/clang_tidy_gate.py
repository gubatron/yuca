"""Prove the configured analyzer rejects a seeded defect, then accepts its repair."""
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    tool, configuration = sys.argv[1:]
    with tempfile.TemporaryDirectory(prefix="tidy-gate-", dir=Path.cwd()) as directory:
        source = Path(directory) / "integer_division.cpp"
        command = [tool, f"--config-file={configuration}", str(source), "--", "-std=c++11"]
        source.write_text("double ratio(int a, int b) { return a / b; }\n", encoding="utf-8")
        broken = subprocess.run(command, text=True, capture_output=True, timeout=20)
        if broken.returncode == 0 or "bugprone-integer-division" not in broken.stdout + broken.stderr:
            raise RuntimeError(f"analysis gate failed to reject seeded defect:\n{broken.stdout}{broken.stderr}")
        source.write_text("double ratio(int a, int b) { return static_cast<double>(a) / b; }\n", encoding="utf-8")
        repaired = subprocess.run(command, text=True, capture_output=True, timeout=20)
        if repaired.returncode != 0:
            raise RuntimeError(f"analysis gate rejected corrected fixture:\n{repaired.stdout}{repaired.stderr}")
    print("clang-tidy rejected integer division and accepted the C++11 correction")


if __name__ == "__main__":
    main()
