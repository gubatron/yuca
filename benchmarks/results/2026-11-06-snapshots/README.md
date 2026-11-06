# .yuca checkpoint strategy and qualification measurements

Feature revision: [`fdd6105`](https://github.com/gubatron/yuca/commit/fdd6105a1d1d2d90e1cf1b350ada50ab747fccb7). Parent/native baseline: `2bb18d6`. Darwin arm64, AppleClang 21/libc++, C++11, CMake Release `-O3 -DNDEBUG`, no sanitizers or allocation overrides in timed executables. Exact hardware/compiler/flags, final executable SHA-256, source revisions and commands are in `environment.json`.

## Final v1 checkpoint results

Each row below is the median of seven recorded samples after warmup, using `yuca_snapshot_benchmark`. Corpus setup, output validation, query checks, and result destruction are outside timers. Bulk/replay decoding alternates order within each sample. File cache is warm; save includes encoding, file data sync and rename, **not** a parent-directory fsync. These are not power-loss durability or latency guarantees.

| Documents | Snapshot bytes | Encode ms | Bulk decode ms | Replay decode ms | Save ms | Load ms |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1,000 | 294,854 | 0.193 | 0.520 | 0.761 | 0.670 | 0.756 |
| 10,000 | 2,957,954 | 1.467 | 4.729 | 6.217 | 2.611 | 5.202 |
| 100,000 | 29,678,954 | 14.641 | 59.377 | 83.455 | 42.428 | 66.222 |

Raw files: `v1-{1000,10000,100000}.csv`; machine-readable medians: `summary.json`. The corpus has three keys in two groups and all five property types, not arbitrary user documents or a million-document workload. Larger documents/properties/groups change both file size and reconstruction cost. The bulk loader avoids public mutation's per-document transaction snapshots while retaining identical parse/validation/round-trip checks; replay is a private benchmark seam, not a second public file format.

## Strategy experiments and historical development runs

- `payload-initial-*.csv` and `payload-v1-*.csv`: `yuca_snapshot_strategies` compares inline strings with a global dictionary (u64 references), on identical ordered string records with independent decode/record equality checks. These payloads are **not .yuca files** and exclude the production format's numeric properties/header. Dictionary encoding reduces size (13,578,898 to 9,790,379 bytes at 100,000 records) but its map construction/lookups are much slower to encode on this corpus. Initial 100,000-record medians were 3.83 ms inline versus 112.51 ms dictionary encode; decode was 7.89 versus 6.67 ms. Inline was selected for checkpoint latency/simplicity, not minimum disk size or universal superiority.
- `fnv-development-*.csv`: earlier, uncommitted production-code prototype using byte-at-a-time FNV-1a and per-group sort-vector allocations. The 100,000-document encode median was 44.92 ms. These are historical observations, not a supported alternate file version or an exact benchmark reproducible with today's executable.
- `xxh-development-100000.csv`: intermediate XXH64 integrity pass before final scratch reuse/validation refinements; encode median 20.40 ms. `qualification-*.csv` retains a later, pre-final qualification. The raw stages are kept to avoid selectively showing only the fastest run. Stage differences combine changes and host drift; **do not infer isolated checksum speedup percentages** from them.

The final portable XXH64 implementation has independent upstream v0.8.3 vectors and a local reference comparison (all lengths 0..4096 at eight alignments, plus 1 MB). Its full BSD-2-Clause notice is in `docs/third-party-licenses.md`. It is an accidental-corruption checksum, not authentication. No new external runtime dependency, compression, mmap, or journal is introduced.

## Native index/search regression control

`core-regression/` compares archived/rebuilt parent `2bb18d6` against the feature using the existing calibrated, interleaved benchmark runner, matching output fingerprints. The seven-sample run measured index -0.31% (bootstrap interval -5.68%..+3.97%) and broad top-ten search +3.53% (+1.23%..+5.34%). The latter prompted a larger-budget confirmation rather than a claim that unchanged search code had regressed.

`core-confirmation/` uses nine samples and a 50 ms target with higher iteration cap: index -0.45% (-4.63%..+5.48%), broad search -1.58% (-4.65%..+2.38%). `core-aa/` runs the identical candidate binary in both slots: index +0.13% (-4.20%..+2.96%), search -1.77% (-5.77%..+4.25%). Preserve all runs: the initial small search signal was not reproduced, and no portable native speedup/regression is established. These bootstrap intervals quantify sampled variation, not systematic host drift.

## Limits and reproduction

The host was not dedicated, thermally controlled, or fixed to a measured power mode. Earlier qualification overlapped other verification work; final v1/payload series ran serially. File-sync timing is notably sensitive: final 100,000-document saves ranged 19.19–43.21 ms; an earlier qualification median was 21.08 ms versus the final 42.43 ms. No cold-cache, tail-latency, resource-limit, million-document, cross-platform performance, or shutdown-time guarantee is implied.

Commands and exact byte-layout/validation contracts are in `docs/development.md#snapshot-format-and-performance`. `yuca_snapshot_benchmark`'s optional file path is **disposable: it is overwritten and removed**. Never benchmark against an actual backup. CI archives a small `snapshots.csv` and displays its medians directly in the Release benchmark job summary; no noisy timing threshold blocks CI.
