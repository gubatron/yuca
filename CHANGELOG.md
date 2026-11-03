# Changelog

## Unreleased

### Fixed
- Build the vendored Catch runner on Apple Silicon.
- Replace documents by ID without retaining old postings; remove using cached keys, including ID -1; define null-document behavior.
- Prevent dangling container references from temporaries, invalid list insertion, overflowing/limit-sized sublist loops, invalidated self-operation iterators, and termination through incorrect `noexcept` declarations.
- Ignore repeated query whitespace, retain fractional Levenshtein ranking bonuses, and avoid signed overflow in search result IDs.
- Fix division by zero and the excluded upper endpoint in `utils::maxRand`; support the full `size_t` bound range with a standard uniform distribution.

### Added
- Out-of-source, target-based CMake, CTest integration, library/header installation, and optional ASan/UBSan builds.
- Deterministic Release benchmarks, CSV comparison tooling, and checked-in per-fix measurements.
- A versioned C++ engineering skill, offline fallback, and explicit MentisDB synchronization protocol.
- Instrumented libFuzzer targets with differential distance/index oracles, seed corpora, bounded mutation runs, and CI checks with benchmark artifacts.

### Performance
- Update posting lists in place rather than copying the entire document set on insertion, existence checks, and removal. Public result sets remain independent snapshots.
- Sort search results in place, compare by reference, and partially sort only the requested top-k. Equal scores are deterministically ordered by document ID.
