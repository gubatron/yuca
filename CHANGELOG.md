# Changelog

## Unreleased

### Fixed
- Pin Linux fuzz CI to Ubuntu 24.04, matching the compiler/analysis jobs instead of following the Ubuntu 26 `ubuntu-latest` migration.
- Remove artifact upload's Node.js 20 deprecation notice using SHA-pinned `actions/upload-artifact` v7.0.2 (Node.js 24).
- Remove checkout's Node.js 20 deprecation notice using SHA-pinned `actions/checkout` v7.0.1 (Node.js 24).
- Preserve logical index state across allocation failures in map/posting/document insert, replace, and remove; make copied index/document state independent.
- Build the vendored test runner with runtime-sized `SIGSTKSZ`; test hash identity without assuming standard-library hash ordering.
- Build the vendored Catch runner on Apple Silicon.
- Replace documents by ID without retaining old postings; remove using cached keys, including ID -1; define null-document behavior.
- Prevent dangling container references from temporaries, invalid list insertion, overflowing/limit-sized sublist loops, invalidated self-operation iterators, and termination through incorrect `noexcept` declarations.
- Ignore repeated query whitespace, retain fractional Levenshtein ranking bonuses, and avoid signed overflow in search result IDs.
- Fix division by zero and the excluded upper endpoint in `utils::maxRand`; support the full `size_t` bound range with a standard uniform distribution.

### Added
- Isolated phase/scaling benchmark suite with calibrated interleaved comparisons, raw samples, output fingerprints, bootstrap uncertainty, runner tests, and CI artifacts.
- Richer differential fuzz properties for multi-term unions/intersections, duplicate query terms, exact ranked top-k prefixes, maximum limits, and index-copy isolation.
- Bounded allocation-failure sweeps, live-byte leak checks after each attempt, and independent index copy/move tests.
- Native CPU-profile workloads, isolated new/delete allocation metrics and peak RSS, tracker correctness tests, and CI allocation artifacts.
- Isolated Clang source/branch coverage builds, fresh-profile core-only HTML/JSON reports, reporting safeguard tests, and archived CI coverage artifacts.
- Blocking clang-tidy/Clang Static Analyzer checks with a seeded-defect gate test, strict core warnings, and full Linux GCC/Clang unit-test CI.
- Out-of-source, target-based CMake, CTest integration, library/header installation, and optional ASan/UBSan builds.
- Deterministic Release benchmarks, CSV comparison tooling, and checked-in per-fix measurements.
- A versioned C++ engineering skill, offline fallback, and explicit MentisDB synchronization protocol.
- User and development manuals with an executable CTest-verified lifecycle example and explicit ownership, query, ranking, security, and compatibility limitations.
- Instrumented libFuzzer targets with differential distance/index oracles, seed corpora, bounded mutation runs, and CI checks with benchmark artifacts.

### Performance
- Update posting lists in place rather than copying the entire document set on insertion, existence checks, and removal. Public result sets remain independent snapshots.
- Sort search results in place, compare by reference, and partially sort only the requested top-k. Equal scores are deterministically ordered by document ID.
