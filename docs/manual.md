# Yuca user manual

## Contents

- [Purpose and current limits](#purpose-and-current-limits)
- [Build, link, and run](#build-link-and-run)
- [Documents and ownership](#documents-and-ownership)
- [Keys, groups, and queries](#keys-groups-and-queries)
- [Ranking and limits](#ranking-and-limits)
- [Saving and restoring .yuca checkpoints](#saving-and-restoring-yuca-checkpoints)
- [API guide](#api-guide)
- [Troubleshooting](#troubleshooting)
- [Maintenance and next steps](#maintenance-and-next-steps)

## Purpose and current limits

Yuca is a small, in-memory C++11 exact-key search library. Applications supply documents, searchable keys, optional key groups, and unindexed typed properties. An inverted index maps each group's keys to matching documents. No service, database, model download, or runtime dependency beyond the C++ standard library is required.

This is **not yet** a full-text analyzer or a transactional durable search database. It supports whole-index `.yuca` checkpoints, not a write-ahead log or automatic per-write persistence. It has no phrase/proximity search, stemming, Unicode normalization, BM25 statistics, embeddings, approximate nearest-neighbor index, or thread-safe writer. There is currently no public C ABI; the native interface is C++, with experimental legacy Java/SWIG wrappers.

## Build, link, and run

Requirements: CMake 3.16+, a C++11 compiler and standard library, and Make/Ninja or another supported generator. The current locally verified platform is macOS arm64 (AppleClang 21 and Homebrew LLVM 23). CI includes full Linux GCC/Clang unit tests and Clang differential fuzz tests. MSVC and a complete cross-architecture/standard-library matrix are not yet established.

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel 4
ctest --test-dir build/release --output-on-failure
cmake --install build/release --prefix "$PWD/build/stage"
```

The complete [basic example](examples/basic.cpp) creates and searches a grouped document, replaces it, and removes it. It is built and run by CTest as `manual_example`. Its expected output is:

```text
42 hello yuca
```

To compile it directly against the staged static library on macOS:

```sh
c++ -std=c++11 -I build/stage/include docs/examples/basic.cpp build/stage/lib/libyuca_static.a -o build/manual_example
build/manual_example
```

On platforms using `lib64`, substitute the actual installed library directory. Use the platform's library suffix/toolchain on Windows. To embed with CMake, use `add_subdirectory(path/to/yuca)` and `target_link_libraries(your_target PRIVATE yuca::static)` or `yuca::shared`. Installation currently provides headers and libraries, not a `find_package` configuration. Shared-library consumers must arrange runtime library discovery using their platform's loader conventions.

## Documents and ownership

1. Construct a `Document` with a `long` ID. Numeric IDs are application-managed. String IDs are converted using `std::hash<std::string>`: collisions are possible and values are not a portable persistent format.
2. Add each searchable value as a `StringKey(value, group)`. Keys are exact, case-sensitive strings. Values containing spaces are single keys, not automatically tokenized.
3. Add unindexed properties using `boolProperty`, `byteProperty`, `intProperty`, `longProperty`, and `stringProperty` setters. These are returned with matches but do not participate in retrieval.
4. Call `indexDocument(document)` to retain a copy, or `indexDocument(shared_ptr)` to retain that same pointer. Reusing an ID replaces the indexed document and removes its previous postings.
5. Remove the indexed document **before mutating keys on a retained pointer**, including a pointer obtained from a search result. Reindex only after removal and mutation. Otherwise the document and posting lists can disagree. The value overload avoids input aliasing but result pointers still expose stored mutable documents.
6. Remove by ID or equivalent document, or call `clear()` to release index-held references. Existing result objects can keep documents alive through shared ownership.

Result collections are independent snapshots of membership, not immutable snapshots of document content. Clearing or replacing an index does not invalidate a result's shared document pointer. `getDocument` returns a value copy; missing IDs return `Document::NULL_DOCUMENT` (ID -1). Because ID -1 is also accepted as a real ID, equality with the sentinel is ambiguous for that ID; prefer application IDs other than -1. Removal itself uses actual cache presence and works for -1.

Indexer instances have no internal locking. Externally synchronize accesses when any thread mutates an instance or its retained documents. `shared_ptr` only manages lifetime, not pointee synchronization. Indexer copy construction creates independent document values, key objects, and reverse indexes, preserving the implicit group. Clearing or editing the copied index does not mutate the original. Copying rebuilds the index and can allocate/throw. Ordinary Document value copies still share key pointers; treat retained key objects as immutable. Move construction is supported; copy assignment remains unavailable because the implicit group is const.

## Saving and restoring .yuca checkpoints

```cpp
yuca::Indexer index(":title");
// ...index documents and typed properties...
index.save("catalog.yuca"); // Explicitly replaces this path on success.

// On the next session, construct a new independent index in one operation:
auto resumed = yuca::Indexer::load("catalog.yuca");
// resumed.indexDocument(...), resumed.removeDocument(...), resumed.search(...)
resumed.save("catalog.yuca");

// The identical format is also available as a contiguous in-memory byte blob:
auto bytes = resumed.serialize(); // std::vector<unsigned char>
auto cloned = yuca::Indexer::deserialize(bytes);
// Or deserialize(pointer, byte_count, optional_limits) for caller-owned buffers.
```

The [complete persistence example](examples/persistence.cpp) is compiled/tested against both static and shared libraries. Run `build/release/yuca_persistence_example_static build/example.yuca`; it reports `2 documents restored; incremental checkpoint verified` and leaves the example checkpoint at that explicitly supplied path.

**What is saved:** the implicit query group, every numeric document ID (including negative IDs), every group's key strings/IDs, and all bool/byte/int/long/string properties, including explicitly stored default values, empty strings, and embedded NUL bytes. Empty documents are retained. Key order is canonicalized, so pointer allocation and insertion order do not change the bytes. Existing query result objects, random request/result IDs, allocator internals, pointers, and transient query state are not persisted. The loader bulk-builds new document/property/key storage and reverse postings; it neither aliases the source index nor needs the input buffer after returning. Further insertions, replacements, removals, searches, and checkpoints work normally.

**Consistency and errors:** externally synchronize all index/retained-document access while saving; there is no concurrent-writer snapshot isolation. Observe the existing rule to remove a document before mutating its keys. `deserialize`/`load` return a new Indexer, rather than partially modifying an existing one; on invalid input they throw `SnapshotError`. File creation/read/write/sync/rename failures also throw `SnapshotError`; allocation failure propagates as `std::bad_alloc`. The const implicit group makes assignment unavailable: construct the returned object, or build a new `unique_ptr<Indexer>` before swapping an application's pointer.

`save` encodes first, creates an exclusive same-directory temporary file (mode 0600 on POSIX), writes and syncs the entire file, closes it, then renames/replaces the destination. A failure before publication leaves the previous file untouched; temporary cleanup is attempted on exception. Parent directories are not created. Use a local filesystem with atomic same-directory rename semantics; network filesystem semantics can differ. This is an atomic checkpoint replacement, **not a guarantee of directory-entry durability across sudden power loss** (the POSIX parent directory is not fsynced), a journal, or a substitute for redundant backups. Saving needs space for both the old and new files plus a contiguous encoded buffer in RAM. Loading temporarily retains both file bytes and the new index. Keep periodic checkpoints rather than relying solely on shutdown callbacks; a killed process cannot run them.

**Compatibility and bounds:** `.yuca` v1 is a versioned little-endian logical format, not a memory dump. It is uncompressed and not encrypted. Because current key/string-document identity relies on implementation-defined `std::hash`, v1 checks the runtime hash ABI and each decoded key ID, rejecting incompatible standard-library/toolchain hashes rather than silently changing retrieval. Reuse a compatible application build; for migration between incompatible hash implementations, export original application records and reindex. Numeric values must fit the loading platform's `long`/`int`; byte properties retain their eight raw bits (numeric `char` signedness is platform-specific). `.yuca` is a convention, not a required filename suffix. Native libraries remain C++11 and have no new runtime dependency. Rebuild consumers against the new headers; Java/SWIG persistence adapters are not provided.

`SnapshotLimits` defaults to 1 GiB total file bytes, 1,000,000 documents, 16,000,000 keys, 16,000,000 properties, 16 MiB per string, and 1 GiB aggregate decoded text (`max_decoded_string_bytes`). The text budget includes group-name copies owned by each key and reverse index, preventing a large shared on-disk group name from expanding without bound. Counts/lengths are checked before allocation/loops, with global key/property/text budgets, duplicate-name/document rejection, and exact end-of-input validation. Adjust the fields explicitly and pass them to `load`/`deserialize` for larger indexes or tighter untrusted-input limits. These budgets are **not** an exact bound on total process RAM: map nodes, pointers, buffer capacity/geometric growth, and temporary string/hash copies add overhead. Select limits for the host's available RAM. XXH64 detects accidental corruption but is not cryptographic authentication: attackers can recompute it. Authenticate/encrypt files externally if required. The checksum implementation's [BSD-2-Clause notice](third-party-licenses.md) accompanies distributions.

See [development and benchmark notes](development.md#snapshot-format-and-performance) for the byte layout, experiments, actual timings, fuzzing, and fault-injection qualification.

## Keys, groups, and queries

An `Indexer()` uses `:keyword` for unqualified terms; `Indexer(":title")` selects another implicit group. Include the leading colon in group names when using group-qualified queries.

| Query | Meaning |
| --- | --- |
| `hello world` | Match at least one of the exact keys `hello` or `world` in the implicit group |
| `hello :extension txt` | Match `hello` in the implicit group **and** `txt` in `:extension` |
| `:city london paris :type hotel` | Match `london` **or** `paris` in `:city`, **and** `hotel` in `:type` |
| `hello :missing value` | No matches if the requested group/value has none |
| whitespace only | No matches |

Whitespace separates tokens; repeated spaces, tabs, and newlines are ignored. A colon-prefixed token changes the current group; it is not a literal keyword. Groups without following keywords do not impose a filter. There is no quoting/escaping grammar, so multiword keys cannot currently be expressed as one query token. Group names and keywords are case-sensitive. Search never indexes properties automatically.

Internally, keys and string-based document IDs rely on hashes. Hash identity and collision handling need redesign before treating the engine as safe against deliberately adversarial key collisions. Do not use these hashes as security identifiers.

## Ranking and limits

The base score counts matching query keyword occurrences across requested groups. Query duplicates count repeatedly. If the comparison-property argument names a non-empty string property, search adds `1 / LevenshteinDistance(raw_query, property)`, or 1 for an exact match. The comparison includes the **whole raw query**, including group syntax; choose the property/query accordingly. Distance is byte-oriented, not Unicode code-point distance. Scores are not probabilities and are not constrained to [0,1].

Order is descending score, then ascending document ID. `search(query, comparison_property, limit)` returns the first `limit` matches; zero means unlimited. A limited ranking is the corresponding prefix of the full ranking. Very large limits do not trigger limit-sized loops.

Top-K sorting uses O(R log K) comparisons for R scored matches when K < R, but retrieval/scoring still processes all candidates. The current index and query API do not enforce resource budgets. Bound untrusted document/query sizes at the application boundary; especially avoid applying quadratic-time distance ranking to arbitrarily large strings. The current distance helper uses linear scratch space in its first string's byte length.

## API guide

| Operation | Contract |
| --- | --- |
| `Document(long)` / `Document(string)` | Create a document with numeric/hash-derived identity |
| `addKey(StringKey)` / `addKey(shared_ptr<StringKey>)` | Associate a key with its group; pointer overload requires non-null |
| `getGroups`, `getGroupKeys`, `getGroupSPKeys` | Copy group/key membership; pointer-key copies retain shared key objects |
| `removeKey(group, pointer)`, `removeGroup(group)` | Edit a document; remove it from the index first if already indexed; key-pointer removal uses pointer identity |
| typed property setters/getters | Store unindexed attributes; missing defaults are false, zero byte, -1 integer/long, or empty string |
| `propertyKeys(type)` | Return a copied list of property names for that type |
| `indexDocument(document)` | Insert a value copy; replace an existing numeric ID |
| `indexDocument(pointer)` | Retain the pointer; null throws `std::invalid_argument` |
| `getDocument(id)` | Return a copied document or the null sentinel |
| `removeDocument(id/document)` | Remove cached content by identity; bool overloads report presence |
| `removeDocument(pointer)` | Remove by ID using cached keys; null/unknown ID is a no-op |
| `findDocuments(key/pointer-list/request)` | Low-level copied membership lookup; keys must be non-null |
| `search(query[, comparison_property[, limit]])` | Return ordered `List<SearchResult>` with score, document pointer and request pointer |
| `clear()` | Release cached documents and reverse indices; retained results remain alive |

Allocating operations may throw standard allocation/copy exceptions. Document insertion, replacement, and removal prepare required snapshots before mutation and roll back unpublished postings on allocation failure: the previous logical index remains intact. This guarantee assumes callers do not mutate retained document keys or bypass invariants through public low-level containers. Reindexing the same retained pointer is a no-op; remove it before changing its keys. The low-level collection wrappers are intended for binding compatibility; arbitrary generic value types can have weaker assignment guarantees than the shared pointers used by the index. See [failure-path qualification](development.md#allocation-failure-injection).

`SearchResult::id` is a legacy wrapping request/document combination, not globally unique. Do not rely on it for durable deduplication. Use your document's application-managed ID.

## Troubleshooting

| Symptom | Check / remedy |
| --- | --- |
| No results | Confirm exact case, implicit group, leading colons, and whether you indexed keys rather than only properties |
| Multiword key cannot be found | Add/tokenize individual words yourself; queries have no phrase grammar |
| Old keywords remain after edits | Remove a retained document before mutating its keys, then reindex |
| Duplicates or unexpected replacement | Check your document ID assignment and string-hash collisions |
| Missing property looks like a real default | Use `propertyKeys(type)` to distinguish absence from an explicitly stored default |
| JNI build fails after an out-of-source core build | Legacy scripts assume root-directory libraries; wrappers have not been modernized or qualified by this pass |
| Fuzz linker cannot find runtime | Use LLVM with libFuzzer; on macOS see the Homebrew compiler selection in the development manual |

## Maintenance and next steps

Use the [development manual](development.md) for exact test, sanitizer, benchmark, and fuzz commands, and [committed measurements](../benchmarks/results/README.md) for qualified performance evidence. See [CHANGELOG](../CHANGELOG.md) and the offline [C++ engineering skill](../skills/cpp-engineer/SKILL.md).

Remaining foundational work includes collision-safe term/document identity, safer retained-document mutation APIs, modernizing the vendored test runner and bindings, broader compiler/platform coverage, and workload-specific optimization. BM25 will require term frequency, document length, and collection statistics not represented by today's unique key sets. Vector search will require an explicit embedding/dimension/metric/storage contract before choosing exact or approximate retrieval. Neither feature is implemented by this maintenance pass.
