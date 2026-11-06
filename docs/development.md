# Yuca development manual

## Supported baseline

The core is C++11 with no runtime dependencies beyond the standard library. CMake 3.16+ supports out-of-source builds. Tests use vendored Catch 2.2.1 with local compatibility patches for the Apple Silicon debugger trap and runtime-sized alternate signal stacks. `catch_portability` reproduces a non-constant `SIGSTKSZ` on Unix hosts. No network access is required to build. Java/SWIG is a separate legacy build, not covered by core CTest.

## Build and test

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DYUCA_BUILD_BENCHMARKS=ON
cmake --build build/release --parallel 4
ctest --test-dir build/release --output-on-failure
```

`BUILD_TESTING=OFF` omits unit tests; `YUCA_BUILD_DEMOS=OFF` omits demos. The `yuca::shared` and `yuca::static` CMake aliases work when embedding with `add_subdirectory`. Public include paths and the minimum language feature propagate to consumers; compiler warnings do not.

To install into a staging directory:

```sh
cmake --install build/release --prefix "$PWD/build/stage"
```

Headers are installed under `include/yuca`; libraries under the platform's library directory. A `find_package` configuration is not supplied yet.

## Reproducible performance comparisons

Configure Release with `YUCA_BUILD_BENCHMARKS=ON`. Do not measure sanitizer builds. Run on an idle machine using the same compiler, flags, hardware, corpus size, and power mode before and after each change:

```sh
git rev-parse HEAD
uname -a
cmake --build build/release --parallel 4
build/release/yuca_benchmarks 7 1000 > build/before.csv
# Apply one fix, rebuild, and rerun with identical arguments.
build/release/yuca_benchmarks 7 1000 > build/after.csv
python3 benchmarks/compare.py build/before.csv build/after.csv
```

Arguments are the number of samples (1–100) and documents (1–100000). The corpus is deterministic: sequential IDs, one common keyword, one of 100 selective terms, alternating extensions, and a title. Each workload warms up before timed samples. CSV includes nanoseconds per operation and a result-count checksum; a volatile sink prevents unused results being discarded. The comparator refuses inconsistent checksums or workload sets and reports medians and ranges. Positive percentages mean slower. Record the commit, environment, commands, raw CSV, and analysis in change notes; rerun overlapping/noisy results rather than claiming wins.

| Workload | Timed work / denominator |
| --- | --- |
| `index_document` | Construct index, insert corpus, search common key, destroy index / documents |
| `search_grouped` | Twenty union-within-group, intersection-across-group searches / searches |
| `search_top10` | Twenty broad searches limited to ten / searches |
| `search_levenshtein` | Twenty broad searches with title-distance ranking, limited to ten / searches |
| `remove_document` | Rebuild index, remove all documents, destroy index / documents |

Lifecycle timings deliberately include setup/cleanup and are not isolated insertion/removal latency. These are initial microbenchmarks, not representative production guarantees. Checksums detect cardinality changes, not full ranking correctness; unit tests verify semantics. The suite does not yet count allocations or RSS, provide confidence intervals, or cover concurrent access. Use Instruments/Allocations on macOS or `perf`/heaptrack on Linux for deeper evidence. Never make noisy timing thresholds a unit-test gate.

Posting-list insertion/removal modifies the stored set in place. For a posting list containing P document pointers, set insertion/removal is O(log P) rather than O(P) copying plus insertion/removal; key-cache/map lookups add their own logarithmic costs. Public `getDocuments`/`findDocuments` still return copied sets, preserving independent result snapshots. Retrieval and scoring retain other copy/allocation costs; the improvement is not a claim that all indexing/search work is logarithmic.

## Sanitizers

```sh
cmake -S . -B build/sanitize -DCMAKE_BUILD_TYPE=Debug -DYUCA_ENABLE_SANITIZERS=ON
cmake --build build/sanitize --parallel 4
ctest --test-dir build/sanitize --output-on-failure
```

ASan and UBSan instrument library and executable code. LeakSanitizer availability depends on the OS/toolchain; do not interpret lack of leak detection on macOS as a leak-free proof. Sanitizer findings must be fixed, not suppressed globally.

## Collection ownership and failures

The binding-friendly `utils::List`, `Set`, and `Map` wrap standard containers. Their `getStdVector`, `getStdSet`, and `getStdMap` accessors borrow storage only from lvalues. On rvalues they return an owning container, so `for (const auto &key : doc.propertyKeys(STRING).getStdVector())` is safe even in C++11. References from lvalue accessors follow the underlying standard container's invalidation rules. Copy accessors may allocate and throw; generic allocation, comparison, and copy failures propagate rather than terminating through incorrect `noexcept` declarations.

List insertion accepts indices from zero through size, inclusive. Invalid insertion or replacement indices throw `std::out_of_range`. `subList(start, length)` clamps to available elements, returns empty when start is outside the list, and performs work proportional to the returned range—not the requested length. Self-add duplicates a list exactly once; self-remove clears lists and sets.

`utils::maxRand(maxInclusive)` samples the inclusive range [0, maxInclusive], including full-width `size_t` bounds. It adapts the legacy process `rand()` source to `std::uniform_int_distribution` instead of dividing by a potentially zero scale factor. It is not cryptographic and retains global RNG state; sequences may vary across standard-library implementations. Do not use it for security or cross-platform reproducibility.

These accessor signature changes require recompilation of C++ consumers. Legacy SWIG generation must be validated separately before shipping new bindings.

## Search contracts

Queries are whitespace-separated, case-sensitive byte strings. A token beginning with `:` changes the active group for subsequent keywords. The default group is `:keyword`. Repeated whitespace (spaces, tabs, newlines) does not create empty keywords. Explicit-delimiter `utils::split(text, delimiter)` retains its original delimiter-based behavior.

Search unions keyword matches within each requested group and intersects across groups. A missing requested group yields no results. Base score is the number of matching query keywords (repeated query terms count repeatedly); a non-empty comparison property adds `1 / distance`, or 1 for an exact match. The distance is byte-oriented Levenshtein, not Unicode code-point distance. Scores are **not** normalized to [0,1]. A zero result limit means unlimited.

`SearchResult::id` is a legacy non-negative wrapping combination of document and request IDs, not a globally unique identifier. Request IDs still use the process RNG; neither identifier is suitable for security, persistence, or deduplicating unrelated requests. Prefer document IDs for application identity.

Results are ordered by descending score, then ascending document ID. A limited search returns exactly the corresponding prefix of the unlimited ranking. Sorting operates on the result vector in place: O(R log K) comparison work for top-K out of R candidates (when 0 < K < R), otherwise O(R log R). Candidate discovery and scoring still process all matches, so a result limit is not a cap on query CPU or memory. Very large limits are clamped by actual result count without limit-sized loops.

## Coverage-guided fuzz testing

Clang with libFuzzer is required. The option below instruments the **library as well as the harnesses**, with coverage, ASan, and UBSan. It is a separate build, not a benchmark configuration:

```sh
cmake -S . -B build/fuzz -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DYUCA_BUILD_FUZZERS=ON
cmake --build build/fuzz --parallel 4
ctest --test-dir build/fuzz --output-on-failure
mkdir -p build/fuzz/corpus
cp -R fuzz/corpus/utils build/fuzz/corpus/
cp -R fuzz/corpus/indexer build/fuzz/corpus/
UBSAN_OPTIONS=halt_on_error=1 build/fuzz/yuca_fuzz_utils build/fuzz/corpus/utils -max_total_time=60 -max_len=512 -timeout=5 -rss_limit_mb=1024
UBSAN_OPTIONS=halt_on_error=1 build/fuzz/yuca_fuzz_indexer build/fuzz/corpus/indexer -max_total_time=60 -max_len=512 -timeout=5 -rss_limit_mb=1024
```

CTest replays the checked-in seeds without mutation (`-runs=0`); the explicit commands mutate **build-directory copies**, not tracked source seeds. `utils` compares Levenshtein against an independent full-matrix reference, checks whitespace splitting, range bounds (including unsigned maximum), and collection self-operations. `indexer` interprets bounded operation streams and compares insert/replace/remove/clear/grouped-search/top-one results against a simple map model after each step. It also exercises arbitrary-byte queries. Each input is capped at 512 bytes; distance strings at 128 bytes and state at 16 document IDs, so fuzzing spends time exploring behavior rather than inducing unbounded test work.

The index model now gives each document up to two keyword values and generates two-term queries, including duplicate query terms. Its independent reference fully sorts expected IDs/scores and checks exact ordered prefixes, not just membership/counts, for unlimited, top-one, and maximum-unsigned limits. Group intersections, union-within-group scoring, duplicate-term weighting, deterministic ties, and copy/clear independence are therefore exercised by mutation. An explicit copy action verifies the clone before clearing it and rechecking the original. This remains a bounded model, not coverage of arbitrary collection sizes, hash collisions, allocation failure, or concurrent access.

For repeatable bounded smoke runs, replace `-max_total_time=60` with `-runs=10000 -seed=444`. Fuzzing is not proof of correctness; increase budgets and corpora for release qualification. On failure, retain the `crash-*`, `timeout-*`, or sanitizer artifact, reproduce with `build/fuzz/yuca_fuzz_indexer path/to/artifact`, minimize with libFuzzer's `-minimize_crash=1`, and add a named deterministic regression test plus the minimized seed. Ignore rules prevent accidental artifact commits; explicitly add reviewed minimized seeds under `fuzz/corpus/`.

If the linker cannot find the libFuzzer runtime, install a Clang distribution that includes it. Do not silently skip instrumentation or replace the target with a non-fuzz executable.

On macOS, Apple's bundled Clang may omit libFuzzer. Homebrew LLVM can be selected explicitly in a **new** build directory with `-DCMAKE_CXX_COMPILER="$(brew --prefix llvm)/bin/clang++"`; do not change compilers inside an existing CMake cache. Local corpus replay was verified with Homebrew LLVM 23.1.4 as well as core ASan/UBSan tests with AppleClang 21.

Initial bounded local qualification: both fuzzers completed 10,000 runs with seed 444, ASan/UBSan, maximum input 512 bytes, 5-second per-input timeout and 1024 MiB RSS limit, with no finding. This is a smoke budget, not exhaustive security qualification.

Extending the utility harness to random bounds subsequently reproduced division by zero in `maxRand` on the existing text corpus. The fixed implementation is covered by a named deterministic maximum-bound regression and the corpus; new mutation runs are required after the fix. This illustrates why a clean fuzz run only covers the behaviors exercised by its harness.

## Continuous integration

`.github/workflows/native.yml` runs macOS core tests in Release and ASan/UBSan configurations, full Linux GCC/Clang unit tests, blocking Linux static analysis, and instrumented Linux Clang fuzzers. Release benchmark CSV/environment information is archived. CI timings are informational; heterogeneous hosted runners are unsuitable for hard performance gates. Windows/MSVC, Linux libc++, and cross-architecture execution beyond hosted runner architectures remain follow-up work.

## Blocking static analysis and strict warnings

All Linux jobs explicitly use `ubuntu-24.04`, including fuzzing. This avoids the rolling `ubuntu-latest` migration to Ubuntu 26 and keeps fuzz/compiler/coverage environments aligned. Moving to a newer image is a deliberate toolchain qualification change, not an incidental runner-label update. macOS continues to track `macos-latest`; no macOS migration notice was present in the reported run.

Artifact uploads use SHA-pinned `actions/upload-artifact` v7.0.2 (Node.js 24), retaining zipped multi-path uploads and the existing artifact names/conditions. The v7 `archive` default is `true`, so consumers still receive archives; no single-file/unarchived mode is requested. `python3 tests/test_native_workflow.py` checks reviewed action pins and runs in the static-analysis job; actionlint separately validates workflow syntax and expressions.

CI checkout uses `actions/checkout` v7.0.1, pinned to its immutable commit SHA. Its Node.js 24 runtime requires Actions Runner 2.327.1+; these jobs use GitHub-hosted runners. Checkout v7's fork protections do not require opting into unsafe checkout: this workflow uses ordinary `push`/`pull_request`, not privileged `pull_request_target`/`workflow_run` triggers. Keep the version comment and reviewed SHA together when upgrading. Workflow-only maintenance changes no native code or benchmark algorithms; use actionlint and hosted CI to verify action compatibility, not native timing as evidence of a speedup.

The core compiles with `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow` on GCC/Clang. Enable `YUCA_WARNINGS_AS_ERRORS=ON` to reject core compiler warnings. These options are private to Yuca, not requirements imposed on consumers or the vendored test runner.

The optional analysis build needs clang-tidy and Python 3 (for the analyzer gate self-test):

```sh
cmake -S . -B build/analysis -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DYUCA_ENABLE_CLANG_TIDY=ON -DYUCA_WARNINGS_AS_ERRORS=ON
cmake --build build/analysis --parallel 4
ctest --test-dir build/analysis --output-on-failure
```

If the tool is outside PATH, supply `-DYUCA_CLANG_TIDY_EXECUTABLE=/path/to/clang-tidy`. On macOS, Homebrew LLVM provides both tools; select its compiler explicitly in a fresh build directory. Missing tools fail configuration rather than silently disabling the requested checks.

`.clang-tidy` enables the Clang Static Analyzer plus selected bug-prone, portability, and inefficient-operation checks. Findings are errors. The static target is analyzed because static/shared targets compile the same core source; this avoids duplicate reports. `clang_tidy_gate` proves the active configuration rejects integer division incorrectly returned as floating point and accepts the corrected C++11 fixture. Analysis does not replace runtime tests or instrumented fuzzing.

The legacy non-cryptographic `rand()` adapter has one documented, check-specific suppression to preserve `srand` compatibility; it is never suitable for security. Broad style/performance suggestions, ABI-changing parameter rewrites, enum-size changes, and static-initialization advisories require separate review rather than automatic edits. For an advisory full scan, configure `CMAKE_EXPORT_COMPILE_COMMANDS=ON` and run clang-tidy with an explicit broader `--checks` list; do not weaken the blocking gate to hide a real defect.

## Source and branch coverage

`tests/key_tests.cpp` verifies every comparison at minimum/zero/maximum IDs, same-ID/different-group equality, default/getter behavior, and both diagnostic stream overloads including their returned stream and flush. Tests do not rely on hash ordering or pointer addresses. The local Clang coverage run after these additions measured `key.cpp` at 100% lines, branches, functions, and regions (32/32, 4/4, 9/9, 11/11). These are assertion-based test additions only; production code and benchmark algorithms are unchanged, so no performance improvement is claimed.

`tests/indexer_tests.cpp` additionally asserts request/result identity, defensive handling of externally removed/empty public posting maps, empty/base-key/string-key diagnostics, unknown-removal overloads, exact/fractional/missing/empty distance-ranking properties, independent copies with custom groups, and multi-group clear/cache removal/reuse. The clear regression documents GitHub #1, originally fixed in `a1194e5` (May 2018). Local LLVM 23 qualification measured `indexer.cpp` at 379/394 lines (96.19%), 138/148 branches (93.24%), 34/34 functions (100%), and 176/184 regions (95.65%). Production source is unchanged; unreachable defensive branches and uninjected allocation-failure paths are not manufactured or excluded to improve the score.

The coverage target now requires **each** of `key.cpp` and `indexer.cpp` to be strictly above 90% for lines, branches, functions, and regions. The check uses unrounded integer counts, rejects missing/duplicate/uninstrumented records, and runs after saving the reports so regressions remain inspectable in CI artifacts and summaries. This is a per-file gate, not a whole-library percentage promise. Toolchain branch counts can differ; CI's Clang 18 result is authoritative for that environment. Other core files remain informational. Gate tests cover exactly 90%, below-threshold, absent/duplicate records, zero opportunities, and invalid counts.

New workflow runs show a **Core coverage** table directly on the GitHub Actions run Summary page, below the job graph, using `GITHUB_STEP_SUMMARY` (the same mechanism as FrostWire). It lists total/per-source line, branch, function, and region coverage with covered/total counts; zero opportunities display as `—`, not perfect coverage. A direct `core-coverage` artifact link opens the full downloadable report, including `html/index.html`. GitHub does not render that interactive HTML inside a job summary. Historical runs are unchanged. Missing reports are explicitly marked unavailable, and the job status is shown so partial results are not mistaken for a passing run. The summary renderer only reads existing reports and does not rerun tests or change native performance.

Validate the standard-library-only renderer with `python3 tests/test_ci_summary.py`. To preview locally, run `python3 scripts/ci_summary.py coverage build/coverage/coverage > build/coverage-summary.md`.

Use a separate Clang Debug build with matching `llvm-cov` and `llvm-profdata` versions. The normal build remains dependency-free; this optional reporting path also needs Python 3:

```sh
cmake -S . -B build/coverage -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DYUCA_ENABLE_COVERAGE=ON
cmake --build build/coverage --target coverage --parallel 4
```

If tools are outside PATH, pass `YUCA_LLVM_COV` and `YUCA_LLVM_PROFDATA` explicitly. On macOS, select the Homebrew LLVM compiler and tools together; do not mix Homebrew and Xcode profile formats. Sanitizers, fuzzers, and benchmarks are deliberately rejected in this build so results cannot be mistaken for performance measurements or fuzz edge coverage.

The `coverage` target builds test executables, runs CTest with a fresh profile directory, merges profiles, and produces `build/coverage/coverage/core.txt`, `core.json`, `summary.json`, `core.profdata`, and `html/index.html`. Reports select only `src/yuca` sources/headers, excluding Catch, test code, and standard-library implementation code. Existing profiles are never reused. Missing core source instrumentation, absent profiles, invalid counts, failed tests, or failed tools are errors; parser tests exercise these reporting safeguards.

The HTML shows individual branches; summary JSON includes covered/total line, branch, function, and region counts. CI archives the report as `core-coverage`. Initial local LLVM 23 coverage on 2026-11-04 was 826/902 lines (91.57%) and 199/230 branches (86.52%). Toolchain/template instantiation differences can change counts; compare within the same toolchain. The key implementation had the weakest initial line coverage (56.25%). Use these gaps to choose meaningful tests, not to manufacture a 100% score. The later per-file gates described above preserve qualified key/indexer coverage; no whole-library threshold is imposed. Passing coverage does not prove assertions are strong or all behaviors are correct. Mutation testing is not installed by this step.

## CPU and allocation profiling

```sh
cmake -S . -B build/profile -DCMAKE_BUILD_TYPE=RelWithDebInfo -DYUCA_BUILD_PROFILING=ON
cmake --build build/profile --parallel 4
ctest --test-dir build/profile --output-on-failure
build/profile/yuca_workload search_top10 10000 3000
build/profile/yuca_allocations index 1000 1
build/profile/yuca_allocations search_top10 1000 100
```

Both drivers use the same deterministic corpus as the microbenchmarks. Arguments are workload, documents (1–100000), and iterations (1–100000). Workloads are `index`, selective `search`, `search_grouped`, `search_top10`, and `search_distance`. Index runs once; queries run the requested iterations. Corpus creation and initial query-index population occur before the measured allocation window; result construction/destruction is included. Output cardinality is checked before reporting.

Use **`yuca_workload`** for CPU sampling: it uses the normal allocator. For a text call-tree sample on macOS, launch it in one terminal and run `/usr/bin/sample <its-pid> 3 -file build/profile/time-profile.txt` in another. Instruments Time Profiler/Allocations can target the same executable. On Linux, use `perf record -g -- build/profile/yuca_workload search_top10 10000 3000`, then `perf report`; `perf stat` can measure cycles/instructions/branch/cache events when host permissions permit. These tools are optional host tooling, not downloaded or configured by CMake. Do not change machine-wide profiling/security settings just to collect a trace.

**`yuca_allocations`** links a diagnostic new/delete replacement into that executable only. The distributed libraries and CPU driver never link it. JSON reports allocation call count, cumulative requested bytes, live requested bytes before/after, and peak extra live bytes above the starting baseline. It handles normal C++11 alignment, arrays, zero-size/nothrow/sized deletion, and checks tracker accounting in CTest. It counts intercepted C++ new/delete payloads, **not** malloc calls made directly by C libraries, allocator metadata, reserved capacity outside the window, or all process memory. It is intended for single-threaded workload windows; cross-thread activity could contaminate window totals. There is no support for over-aligned C++17 allocation in this C++11 driver.

`peak_process_rss_bytes` is separately obtained from `getrusage` on macOS/Linux, with platform units normalized to bytes; zero means unavailable on unsupported hosts. RSS is a process-wide lifetime high-water mark, including corpus/runtime/diagnostic overhead, not per-query memory or memory returned to the OS. Diagnostic headers/counters perturb allocator behavior and timings. Never use `yuca_allocations` timing as production performance evidence, and do not combine this profiling build with sanitizers/fuzzers/coverage.

Initial local evidence (2026-11-04, AppleClang 21/libc++, 1000 documents): indexing requested 24,208 allocations / 1,637,408 bytes; 100 top-ten searches requested 1,403,000 allocations / 85,111,200 bytes, with no net live-byte growth across the query loop. This is **14,030 new calls per broad query**, not evidence of a leak. A three-second native `sample` trace over 10,000 documents showed set/tree copying and allocation beneath `findDocuments`/`getDocuments` in retrieval. This identifies an optimization target; it does not yet measure a fix or establish a portable CPU percentage. Raw allocation records are under `benchmarks/results/`; the large host-specific call tree remains in `build/profile/`.

## Allocation failure injection

The profiling build also provides `yuca_allocation_failures`. It replaces only that executable's new/delete, arms a one-shot failure after N successful allocation calls, then disarms before checking/reporting. No fault hook is linked into the shipped library. Run all scenarios with `build/profile/yuca_allocation_failures`, or isolate `map`, `posting`, `insert`, `replace`, `remove`, or `copy` as an argument.

Each mutation starts from a fresh fixture, tests every successive allocation failure until the first success, and stops after a bounded 4096 attempts if success is never reached. Assertions verify old cache/posting membership remains intact after failure, successful output is complete, and tracked live bytes return to the pre-fixture baseline after destruction. This is stronger than a single allocation failure at startup; it covers partially completed operations. The hook intentionally bypasses new-handler recovery for injected failures, automatically disarms before throwing, and requires a single-threaded test window. Keep it separate from sanitizers; normal code and copy semantics are independently checked under ASan/UBSan.

The initial sweep reproduced lost map mappings, partially published posting/cache entries, destructive failed replacement, and partial removal. Map updates now retain existing nodes; posting insertion cleans up partial keys; Indexer prepares group/key snapshots and missing reverse indexes before publishing, retains previous postings until replacement succeeds, and uses allocation-free rollback/removal. Existing common-term posting sets are not cloned for transactions. The guarantee applies to the index's standard shared-pointer/string internals, not arbitrary user-defined throwing assignments in the generic Map wrapper.

`indexer_copy` independently verifies copied document/key identity, properties, preserved implicit group, clear independence, and move construction. Indexer copy construction rebuilds independent content and clones key objects. Ordinary Document copies still share key pointers; retained key objects must not be mutated while indexed. Passing these bounded fixtures does not establish arbitrary exception safety for every externally mutated low-level structure or concurrent writer.

Rebuild C++ consumers to use the explicit copy constructor; code compiled against the old header can retain its old inline shallow-copy behavior. The post-fix deterministic coverage run measured 877/958 core lines (91.54%) and 228/266 branches (85.71%) locally. The allocation-failure executable deliberately uses a separate non-coverage allocator build, so its exercised error paths are not included in those coverage totals.

## Inline benchmark reports

The macOS non-sanitized job publishes **Release benchmarks** directly on the run Summary page: microbenchmark medians/ranges, scaling median/bootstrap intervals and calibration-cap flags, and allocation/RSS totals with explicit units. Commit/runner/compiler details are expandable, and a direct `release-benchmarks` link retains the full CSV/JSON samples and fingerprints. Summaries read existing outputs, never time rendering or rerun workloads. Small hosted-runner samples remain informational, not p95/p99 estimates or a regression gate. Missing/incomplete sections are labeled unavailable; job status identifies failed runs. Earlier runs cannot acquire summaries retroactively. Preview with `python3 scripts/ci_summary.py benchmarks build/ci > build/benchmark-summary.md` after generating those reports; test with `python3 tests/test_ci_summary.py`.

## Snapshot format and performance

Public usage and operational limits are in the [checkpoint manual](manual.md#saving-and-restoring-yuca-checkpoints). `src/yuca/snapshot.cpp` is a private codec adapter with read-only access to document storage; it avoids public getter copies during encoding. Only an unpublished loader result uses bulk posting construction. The public mutation path keeps its normal transactional guarantees. `Indexer`'s defaulted move explicitly spells `noexcept(false)` because its const implicit-group string is copied and may allocate; this documents the existing exception behavior rather than falsely promising a no-throw move.

### Version 1 byte layout

Every integer is little endian. Header (40 bytes): magic `YUCAIDX\0` (8 bytes), version (u32, 1), flags (u32, 0), payload length (u64), XXH64 checksum of payload (u64, seed 0), and a runtime `std::hash<std::string>` probe (u64). Unknown versions/flags or incompatible hashes are rejected. The payload is:

1. Implicit group (string), document count (u64).
2. Documents in strictly ascending signed ID order: ID (signed 64-bit), group count (u64), then each group name (string), key count (u64, nonzero), and keys (string value, signed 64-bit key ID). Keys are sorted lexically for stable bytes; repeated identical strings on distinct retained key pointers are preserved. Sorting scratch storage is reused across groups/documents.
3. For each document, the five property maps in BOOL, BYTE, INT, LONG, STRING order: count (u64), then name (string) and value. Numeric values occupy 64 bits; bool is 0/1, byte is 0..255 raw bits, int/long are signed and range checked. String values are strings. Names may repeat across types but not within one map. Group names may not repeat within a document.

A string is a u64 byte length followed by exactly that many bytes, without a terminator. Signed values use their modulo-2^64 two's-complement encoding; decoding avoids implementation-defined out-of-range signed conversions. Empty strings and NUL bytes are valid. No padding, pointers, map implementation layout, or compiler struct layout is written. The loader enforces the payload length, checksum, key IDs, count/length budgets and end-of-input. Hash collisions remain the legacy identity limitation; persistence neither solves nor hides them. Files created with a different hash implementation are deliberately not a portable interchange format.

### Reproducible experiments

```sh
cmake -S . -B build/snapshot-release -DCMAKE_BUILD_TYPE=Release -DYUCA_BUILD_BENCHMARKS=ON -DYUCA_BUILD_PROFILING=ON -DYUCA_WARNINGS_AS_ERRORS=ON
cmake --build build/snapshot-release --parallel 4
ctest --test-dir build/snapshot-release --output-on-failure
build/snapshot-release/yuca_snapshot_strategies 100000 > build/snapshot-strategies.csv
build/snapshot-release/yuca_snapshot_benchmark 100000 build/benchmark-only.yuca > build/snapshot-timings.csv
```

**The benchmark's optional file argument is disposable: it is overwritten and removed. Never point it at a real backup.** Without that argument, file timings are zero/not measured. All tools bound document count to 1..1,000,000. The encoding experiment compares the same ordered string records using inline length-prefixes versus a global dictionary with u64 references; both independently decode and compare records outside the timer. It is a payload experiment, **not an alternate `.yuca` reader**, and its sizes exclude the full index's numeric properties/header. Order alternates, one initial iteration is warmup, and seven raw samples are retained.

The production benchmark uses three keys in two groups plus all five property types per document. It times encoding, direct bulk decode, public-`indexDocument` replay decode (private benchmark seam), and optional actual save/load. Direct/replay decode share the same parsing/validation and alternate order each sample. Corpus construction, output fingerprints/byte comparisons, searches, and destruction are outside timers. Save includes encoding, file data sync, and atomic rename; load includes the buffered file read, checksum, validation, and reconstruction. The filesystem page cache is warm, not deliberately flushed. No compression, mmap, journal, or zero-copy restored pointer graph is claimed. `serialize()` needs an output-sized contiguous allocation; v1 uses geometric buffer growth rather than a second sizing scan. CI runs a small 1000-document file benchmark, archives `snapshots.csv` in `release-benchmarks`, and renders byte counts/median timings on the run Summary page.

Initial AppleClang 21/libc++ Release experiments at 100,000 documents: inline string payload encoding median 3.83 ms / 13.58 MB versus dictionary 112.51 ms / 9.79 MB; dictionary decoding was 6.67 ms versus inline 7.89 ms. The production codec chooses inline strings to favor checkpoint/write latency and simplicity, not because it is the smallest or universally fastest format. A first FNV-1a integrity pass dominated encoding, so it was replaced with portable XXH64 and per-group sort allocation was eliminated. The final v1 run produced 29,678,954-byte snapshots: medians **14.64 ms encode, 59.38 ms bulk decode versus 83.46 ms replay, 42.43 ms save and 66.22 ms load**. Save samples ranged 19.19–43.21 ms; an earlier qualification run's save median was 21.08 ms, demonstrating the sensitivity of synced file I/O to host/filesystem conditions. These are one host's seven-sample observations, not a promised latency, million-document result, cold-storage measurement, or throughput guarantee. Ordered FNV/XXH/reuse development runs can be confounded by host drift; only bulk/replay configurations were interleaved within the same run. Raw data and environment notes are retained under `benchmarks/results/2026-11-06-snapshots/`.

XXH64 is specified by upstream xxHash v0.8.3. The small portable implementation has no unaligned native loads and no external runtime dependency. Empty/short/block/tail/unaligned test vectors were checked against upstream; the local independent reference comparison covered every length 0..4096 at eight byte alignments plus 1 MB. The independent downloaded reference header and verifier remain under `build/`, not part of the distributed library. Its [BSD notice](third-party-licenses.md) is installed alongside the library.

### Persistence qualification

`[snapshot]` Catch tests cover all typed values/defaults, numeric extremes, custom/empty groups, duplicate key pointers, binary strings, canonical bytes, independent buffer lifetimes, incremental operations, every truncation of a nontrivial snapshot, malformed headers/checksums/lengths/counts/booleans/bytes/hashes, duplicate names/documents/groups, integer overflow, trailing bytes, missing files/parents, replacement, rename failure, and configured resource limits, including expanded group-name text amplification. Three fixture-dependent `snapshot_session_*` tests write, resume/update, and verify the checkpoint in separate processes, including string-derived document IDs. File loading validates the fixed header before allocating the full bounded buffer. File API qualification is POSIX locally and in Linux/macOS CI; a Windows narrow-path adapter is provided but not verified by the current CI matrix.

The isolated allocation-failure executable sweeps encode/decode allocation points and verifies source invariants/no leaked tracked bytes. On POSIX, `snapshot_save_failures` injects every successive allocation failure during file saving, checking the old checkpoint is byte-identical after failure and no temporary file survives; it then verifies successful replacement. This does not inject ENOSPC, short writes, kernel errors, or process/power failure, and does not combine diagnostic allocators with sanitizers. Normal memory/file paths are exercised separately under ASan/UBSan. `fuzz/snapshot_fuzz.cpp` round-trips generated indexes, compares ordered search IDs/scores, parses arbitrary bytes under tight limits, and repairs checksums on mutated payloads to exercise deep validation. CI replays/mutates all three corpora; generated corpus files remain under `build/`. Coverage includes internal `.h` checksum code as well as public headers, not just `.cpp`/`.hpp`.

## Isolated scaling benchmarks and controlled comparisons

The original `yuca_benchmarks` CSV interface is retained for existing comparisons. `YUCA_BUILD_BENCHMARKS=ON` also builds `yuca_benchmark_suite`, with isolated phases and a separate standard-library-only Python runner:

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DYUCA_BUILD_BENCHMARKS=ON
cmake --build build/release --parallel 4
build/release/yuca_benchmark_suite search_common 1000 100 10
python3 scripts/benchmark_suite.py --candidate build/release/yuca_benchmark_suite --documents 100,1000,10000 --top-k 0,10 --samples 9 --output build/scaling-experiment
```

The direct arguments are workload, documents, iterations, and top-k. Supported workloads: `index`, `replace`, `remove`, `clear`, `copy`, `search_rare`, `search_common`, `search_grouped`, `search_distance`, `search_empty`. Index/replace/remove measure nanoseconds per document; clear/copy measure per whole-index operation; search measures per query. Zero top-k means unlimited. All numeric arguments are bounded to 100000; documents/iterations must be positive. Rare queries rotate over the 100 term values; common queries match all documents, grouped queries match half, and empty queries match none.

Corpus creation, mutation setup, output validation, and result/index teardown are outside timers. Clear/removal deallocation is intrinsically part of the timed mutation. Copy includes allocating the destination Indexer object, but not destroying it. Query timing includes retrieval/scoring/materialization into an initially empty result, but not caller consumption or result destruction. Each process executes one unrecorded warmup operation and then the recorded iterations. The runner pilots/calibrates iteration count toward `--min-sample-ms` (default 50), capped by `--max-iterations`; a capped run is explicitly labeled. Actual elapsed time is retained, so a target duration is not a claim every sample reached it. Timer overhead matters for extremely fast/no-match queries; it is not subtracted using a speculative constant.

Each iteration checks cardinality, sorted score/ID order, duplicate IDs, finite scores, and relevant mutation outcomes. A non-cryptographic 64-bit fingerprint folds ordered document IDs and exact double score bits from **all** recorded iterations; it is a cheap behavior-change detector, not proof against hash collisions or an independent correctness oracle. Differential fuzzing supplies the stronger reference model. Runs must use the same scenario/iteration count and numerical contract; cross-platform bitwise-identical scores are not promised.

To compare two compatible builds, add `--baseline /path/to/old/yuca_benchmark_suite`. Both execute identical scenarios/iteration counts, with configurations interleaved in seeded shuffled order. A fresh process is used per sample; within-process warmup is explicit. The runner rejects nonfinite/invalid metrics, incorrect operation denominators, inconsistent fingerprints within/across configurations, and unsupported schemas. The output directory must not already exist, preventing accidental overwrite of a prior experiment. A failed run retains partial raw samples but does not publish a successful summary.

Outputs are `raw.csv`, `summary.json`, and `environment.json` (binary paths/SHA-256, platform, Python, exact command). Record compiler/standard-library versions, full build flags, source revisions, hardware/power mode, and cold/warm-cache procedure alongside them; the runner cannot infer/control all of these. Reports include sample count, median, min/max, standard deviation, a seeded 2000-resample 95% bootstrap interval for the median, and, when comparing, a bootstrap interval for relative change. These intervals reflect sampled variation, not systematic host drift or proof of causation. Small CI smoke samples are especially weak; no noisy time threshold gates CI, and p95/p99 latency is not inferred from nine samples. CPU sampling, diagnostic allocation metrics, and benchmark timing remain separate measurements.

`benchmark_statistics` checks interval determinism, invalid data, fingerprint mismatch refusal, and a real same-binary baseline/candidate smoke experiment in an isolated temporary directory. CI archives small scaling runs as informational artifacts. Use a quiet stable host, larger budgets and representative application corpora before accepting production performance claims. Google Benchmark/PGO/SIMD and mutation-testing infrastructure are not dependencies introduced by this step.
