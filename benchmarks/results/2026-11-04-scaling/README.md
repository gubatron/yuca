# Initial isolated scaling baseline

Recorded on 2026-11-04, Darwin arm64, AppleClang 21/libc++, CMake Release defaults, C++11, no sanitizers or diagnostic allocation replacement. Source is the suite in the commit adding this directory; executable SHA-256 and exact command are recorded in `environment.json`.

The experiment contains 45 workload/document-count/top-k scenarios and 225 raw samples: all ten phase/selectivity workloads, 100/1000/10000 documents, top-k 0/10 for query cases, five samples per scenario. Target sample duration was 10 ms with at most 100 iterations; **22 scenarios hit that calibration cap**. These are exploratory baselines, not release qualification or reliable tail-latency estimates. They are candidate-only observations, not before/after speedup claims.

`raw.csv` preserves each measured sample; `summary.json` contains medians, ranges, standard deviation and bootstrap median intervals. Setup, validation and teardown exclusions and operation denominators are specified in `docs/development.md`. The host was not dedicated or thermally/power controlled. Future comparisons should rebuild compatible old/new suite executables with identical flags on a stable host, interleave them with the runner, preserve fingerprints, increase budgets for capped cases, and record all environment controls.
