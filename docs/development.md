# Yuca development manual

## Supported baseline

The core is C++11 with no runtime dependencies beyond the standard library. CMake 3.16+ supports out-of-source builds. Tests use vendored Catch 2.2.1 with a small Apple Silicon debugger-trap patch; no network access is required to build. Java/SWIG is a separate legacy build, not covered by core CTest.

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

For repeatable bounded smoke runs, replace `-max_total_time=60` with `-runs=10000 -seed=444`. Fuzzing is not proof of correctness; increase budgets and corpora for release qualification. On failure, retain the `crash-*`, `timeout-*`, or sanitizer artifact, reproduce with `build/fuzz/yuca_fuzz_indexer path/to/artifact`, minimize with libFuzzer's `-minimize_crash=1`, and add a named deterministic regression test plus the minimized seed. Ignore rules prevent accidental artifact commits; explicitly add reviewed minimized seeds under `fuzz/corpus/`.

If the linker cannot find the libFuzzer runtime, install a Clang distribution that includes it. Do not silently skip instrumentation or replace the target with a non-fuzz executable.

On macOS, Apple's bundled Clang may omit libFuzzer. Homebrew LLVM can be selected explicitly in a **new** build directory with `-DCMAKE_CXX_COMPILER="$(brew --prefix llvm)/bin/clang++"`; do not change compilers inside an existing CMake cache. Local corpus replay was verified with Homebrew LLVM 23.1.4 as well as core ASan/UBSan tests with AppleClang 21.

Initial bounded local qualification: both fuzzers completed 10,000 runs with seed 444, ASan/UBSan, maximum input 512 bytes, 5-second per-input timeout and 1024 MiB RSS limit, with no finding. This is a smoke budget, not exhaustive security qualification.

Extending the utility harness to random bounds subsequently reproduced division by zero in `maxRand` on the existing text corpus. The fixed implementation is covered by a named deterministic maximum-bound regression and the corpus; new mutation runs are required after the fix. This illustrates why a clean fuzz run only covers the behaviors exercised by its harness.

## Continuous integration

`.github/workflows/native.yml` runs macOS core tests in Release and ASan/UBSan configurations, archives Release benchmark CSV/environment information, and runs instrumented Clang fuzzers on Linux. CI timings are informational; heterogeneous hosted runners are unsuitable for hard performance gates. Legacy Catch portability on modern Linux and a full GCC/MSVC unit-test matrix remain follow-up work; Linux's initial job exercises library behavior through differential fuzz oracles instead.
