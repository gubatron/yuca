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

These accessor signature changes require recompilation of C++ consumers. Legacy SWIG generation must be validated separately before shipping new bindings.

## Search contracts

Queries are whitespace-separated, case-sensitive byte strings. A token beginning with `:` changes the active group for subsequent keywords. The default group is `:keyword`. Repeated whitespace (spaces, tabs, newlines) does not create empty keywords. Explicit-delimiter `utils::split(text, delimiter)` retains its original delimiter-based behavior.

Search unions keyword matches within each requested group and intersects across groups. A missing requested group yields no results. Base score is the number of matching query keywords (repeated query terms count repeatedly); a non-empty comparison property adds `1 / distance`, or 1 for an exact match. The distance is byte-oriented Levenshtein, not Unicode code-point distance. Scores are **not** normalized to [0,1]. A zero result limit means unlimited.

`SearchResult::id` is a legacy non-negative wrapping combination of document and request IDs, not a globally unique identifier. Request IDs still use the process RNG; neither identifier is suitable for security, persistence, or deduplicating unrelated requests. Prefer document IDs for application identity.
