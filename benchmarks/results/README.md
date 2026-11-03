# Initial maintenance measurements — 2026-11-03

Environment: local Darwin arm64, AppleClang 21.0.0.21000334, libc++, C++11, CMake Release defaults (`-O3 -DNDEBUG`), no sanitizers, single-threaded. Command: `build/release/yuca_benchmarks 7 1000`. Hardware model/power mode were not pinned; treat these as exploratory local evidence, not portable claims or statistically established speedups. Raw CSV is committed alongside this file.

| Change | Baseline | Candidate | Interpretation |
| --- | --- | --- | --- |
| Document replacement/removal correctness | `3900389` | lifecycle fix in the commit adding `2026-11-03-lifecycle.csv` | Index lifecycle +0.37%; removal lifecycle -3.43%; search -1.14% to -5.65%. Ranges overlap: no established regression or improvement. Correctness is the reason to ship. |
| Collection lifetimes, bounds, and exception contracts | `96225e5` | collection fix in the commit adding `2026-11-03-collections.csv` | Index lifecycle -3.64%; grouped search -8.89%; top-ten -10.61%; distance search -7.88%; removal lifecycle -1.14%. Seven local samples suggest reduced search overhead; repeat on target hardware before generalizing. ASan's reproduced stack-use-after-scope is eliminated. |

Compare any adjacent pair using `python3 benchmarks/compare.py before.csv after.csv`. Search and lifecycle correctness are separately verified with regression tests; the benchmark checksums only verify result cardinalities.
