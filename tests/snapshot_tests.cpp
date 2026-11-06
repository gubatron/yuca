#include "catch.hpp"
#include "yuca/yuca.hpp"
#include "../src/yuca/snapshot_checksum.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace {
    using Bytes = std::vector<unsigned char>;
    void put64(Bytes &bytes, std::size_t offset, std::uint64_t value) {
        for (unsigned i = 0; i < 8; ++i) { bytes.at(offset + i) = static_cast<unsigned char>(value); value >>= 8; }
    }
    void append64(Bytes &bytes, std::uint64_t value) { const auto offset = bytes.size(); bytes.resize(offset + 8); put64(bytes, offset, value); }
    void appendText(Bytes &bytes, const std::string &value) { append64(bytes, value.size()); bytes.insert(bytes.end(), value.begin(), value.end()); }
    void repair(Bytes &bytes) {
        put64(bytes, 16, bytes.size() - 40);
        put64(bytes, 24, yuca::detail::snapshotChecksum(bytes.data() + 40, bytes.size() - 40));
    }
    // Hand-build a document payload, independent of the production encoder.
    Bytes scalarFixture(std::uint64_t bool_value, std::uint64_t byte_value) {
        auto bytes = yuca::Indexer().serialize(); bytes.resize(40);
        appendText(bytes, ":keyword"); append64(bytes, 1);
        append64(bytes, 7); append64(bytes, 0); // ID, no key groups
        append64(bytes, 1); appendText(bytes, "flag"); append64(bytes, bool_value);
        append64(bytes, 1); appendText(bytes, "byte"); append64(bytes, byte_value);
        for (unsigned i = 0; i < 3; ++i) append64(bytes, 0);
        repair(bytes); return bytes;
    }
    yuca::Indexer fixture() {
        yuca::Indexer index(":title");
        for (long id : {std::numeric_limits<long>::min(), -1L, 7L, std::numeric_limits<long>::max()}) {
            yuca::Document document(id);
            document.addKey(yuca::StringKey("hello", ":title"));
            document.addKey(yuca::StringKey("pdf", ":extension"));
            document.boolProperty("false", false); document.boolProperty("true", true);
            document.intProperty("minimum", std::numeric_limits<int>::min());
            document.intProperty("maximum", std::numeric_limits<int>::max());
            document.longProperty("minimum", std::numeric_limits<long>::min());
            document.longProperty("maximum", std::numeric_limits<long>::max());
            document.stringProperty("title", "hello");
            document.stringProperty(std::string("na\0me", 5), std::string("va\0lue", 6));
            index.indexDocument(document);
        }
        return index;
    }
}

TEST_CASE("Snapshot XXH64 checksum matches independent standard vectors", "[snapshot]") {
    REQUIRE(yuca::detail::snapshotChecksum(nullptr, 0) == 0xef46db3751d8e999ULL);
    const auto *a = reinterpret_cast<const unsigned char *>("a");
    REQUIRE(yuca::detail::snapshotChecksum(a, 1) == 0xd24ec4f1a98c6e5bULL);
    const auto *abc = reinterpret_cast<const unsigned char *>("abc");
    REQUIRE(yuca::detail::snapshotChecksum(abc, 3) == 0x44bc2cf5ad770999ULL);
    // Vectors generated independently with upstream xxHash v0.8.3, seed 0;
    // exercise the 32-byte accumulator and every tail lane, including unaligned input.
    const std::pair<std::size_t, std::uint64_t> vectors[] = {
        {31, 0x44cc9efe5d2d0233ULL}, {32, 0x0e1aab1d173cf196ULL}, {33, 0x5c820b4b4fe28fdcULL},
        {63, 0x1153d36cade87066ULL}, {64, 0xd4c20ef54cbc9f67ULL}, {65, 0xac086c9b87baea67ULL},
        {256, 0xfa7639c7a1c6e86eULL}, {1000, 0x9fb3251bef67c2b5ULL}};
    std::vector<unsigned char> input(1001);
    for (std::size_t i = 0; i < 1000; ++i) input[i + 1] = static_cast<unsigned char>(i * 131 + 17);
    for (const auto &entry : vectors) REQUIRE(yuca::detail::snapshotChecksum(input.data() + 1, entry.first) == entry.second);
}

TEST_CASE("Snapshots preserve all document values and independent incremental search state", "[snapshot]") {
    auto source = fixture();
    const auto bytes = source.serialize();
    auto loaded = yuca::Indexer::deserialize(bytes);
    REQUIRE(loaded.serialize() == bytes);
    const auto results = loaded.search("hello :extension pdf", "title", 2);
    REQUIRE(results.size() == 2);
    REQUIRE(results.get(0).document_sp->getId() == std::numeric_limits<long>::min());
    REQUIRE(results.get(0).score > 2.0);
    REQUIRE(results.get(1).document_sp->getId() == -1);
    const auto document = loaded.getDocument(7);
    REQUIRE_FALSE(document.boolProperty("false")); REQUIRE(document.boolProperty("true"));
    REQUIRE(document.propertyKeys(yuca::BOOL).size() == 2);
    REQUIRE(document.intProperty("minimum") == std::numeric_limits<int>::min());
    REQUIRE(document.intProperty("maximum") == std::numeric_limits<int>::max());
    REQUIRE(document.longProperty("minimum") == std::numeric_limits<long>::min());
    REQUIRE(document.longProperty("maximum") == std::numeric_limits<long>::max());
    REQUIRE(document.stringProperty(std::string("na\0me", 5)) == std::string("va\0lue", 6));
    REQUIRE(loaded.search("hello").get(0).document_sp != source.search("hello").get(0).document_sp);
    REQUIRE(loaded.removeDocument(-1));
    yuca::Document replacement(7); replacement.addKey(yuca::StringKey("new", ":title"));
    loaded.indexDocument(replacement);
    REQUIRE(loaded.search("hello").size() == 2); REQUIRE(loaded.search("new").size() == 1);
    REQUIRE(source.search("hello").size() == 4);
    auto resumed = yuca::Indexer::deserialize(loaded.serialize());
    REQUIRE(resumed.search("hello").size() == 2); REQUIRE(resumed.search("new").size() == 1);
    resumed.clear(); REQUIRE(resumed.search("hello").isEmpty());
    auto temporary = source.serialize();
    auto independent = yuca::Indexer::deserialize(temporary);
    temporary.clear(); temporary.shrink_to_fit();
    REQUIRE(independent.serialize() == bytes); // No views into the caller's buffer survive.
}

TEST_CASE("Snapshots retain empty documents duplicate key pointers and every byte pattern", "[snapshot]") {
    yuca::Indexer empty("");
    auto restored_empty = yuca::Indexer::deserialize(empty.serialize());
    REQUIRE(restored_empty.search("anything").isEmpty());
    yuca::Document document(7);
    const std::string value("a\0b", 3), group(":g\0x", 4);
    document.addKey(yuca::StringKey(value, group)); document.addKey(yuca::StringKey(value, group));
    for (unsigned i = 0; i < 256; ++i) {
        const auto bits = static_cast<unsigned char>(i); char byte; std::memcpy(&byte, &bits, 1);
        document.byteProperty(std::to_string(i), byte);
    }
    empty.indexDocument(document); empty.indexDocument(yuca::Document(8));
    auto loaded = yuca::Indexer::deserialize(empty.serialize());
    REQUIRE(loaded.getDocument(8).getId() == 8);
    REQUIRE(loaded.getDocument(7).getGroupSPKeys(group).size() == 2);
    REQUIRE(loaded.findDocuments(std::make_shared<yuca::StringKey>(value, group)).size() == 1);
    for (unsigned i = 0; i < 256; ++i) REQUIRE(static_cast<unsigned char>(loaded.getDocument(7).byteProperty(std::to_string(i))) == i);
    REQUIRE(loaded.serialize() == empty.serialize());
}

TEST_CASE("Snapshot bytes are canonical across insertion and pointer order", "[snapshot]") {
    yuca::Indexer first, second;
    for (long id = 1; id <= 3; ++id) {
        yuca::Document a(id), b(4 - id);
        for (const auto &word : {"alpha", "beta"}) a.addKey(yuca::StringKey(word, ":keyword"));
        for (const auto &word : {"beta", "alpha"}) b.addKey(yuca::StringKey(word, ":keyword"));
        first.indexDocument(a); second.indexDocument(b);
    }
    REQUIRE(first.serialize() == second.serialize());
}

TEST_CASE("Snapshot reader rejects every truncation and header or payload corruption", "[snapshot]") {
    const auto bytes = fixture().serialize();
    for (std::size_t length = 0; length < bytes.size(); ++length) REQUIRE_THROWS_AS(yuca::Indexer::deserialize(bytes.data(), length), yuca::SnapshotError);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(nullptr, bytes.size()), yuca::SnapshotError);
    for (std::size_t offset : {0U, 8U, 12U, 16U, 24U, 32U, 40U}) {
        auto broken = bytes; broken[offset] ^= 1;
        REQUIRE_THROWS_AS(yuca::Indexer::deserialize(broken), yuca::SnapshotError);
    }
    auto trailing = bytes; trailing.push_back(0); repair(trailing);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(trailing), yuca::SnapshotError);
    auto invalid_string = bytes; put64(invalid_string, 40, std::numeric_limits<std::uint64_t>::max()); repair(invalid_string);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(invalid_string), yuca::SnapshotError);
    auto huge_count = bytes; put64(huge_count, 54, std::numeric_limits<std::uint64_t>::max()); repair(huge_count); // after ":title"
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(huge_count), yuca::SnapshotError);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(scalarFixture(2, 0)), yuca::SnapshotError);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(scalarFixture(1, 256)), yuca::SnapshotError);
    auto valid = yuca::Indexer::deserialize(scalarFixture(1, 255));
    REQUIRE(valid.getDocument(7).boolProperty("flag"));
    REQUIRE(static_cast<unsigned char>(valid.getDocument(7).byteProperty("byte")) == 255);
}

TEST_CASE("Snapshot resource limits are checked independently", "[snapshot]") {
    const auto bytes = fixture().serialize();
    yuca::SnapshotLimits limits;
    SECTION("bytes") { limits.max_bytes = bytes.size() - 1; }
    SECTION("documents") { limits.max_documents = 3; }
    SECTION("keys") { limits.max_keys = 7; }
    SECTION("properties") { limits.max_properties = 1; }
    SECTION("string bytes") { limits.max_string_bytes = 3; }
    SECTION("expanded text") { limits.max_decoded_string_bytes = 3; }
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(bytes, limits), yuca::SnapshotError);
}

TEST_CASE("Snapshot text budget counts group expansion rather than just on-disk strings", "[snapshot]") {
    yuca::Indexer index;
    yuca::Document document(7);
    const std::string group(64, 'g');
    for (const auto &word : {"one", "two", "three"}) document.addKey(yuca::StringKey(word, group));
    index.indexDocument(document);
    const auto bytes = index.serialize();
    auto loaded = yuca::Indexer::deserialize(bytes);
    REQUIRE(loaded.getDocument(7).getGroupSPKeys(group).size() == 3);
    yuca::SnapshotLimits limits;
    limits.max_decoded_string_bytes = 256;
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(bytes, limits), yuca::SnapshotError);
}

TEST_CASE("Snapshot duplicate documents are rejected without altering the source", "[snapshot]") {
    yuca::Indexer source;
    source.indexDocument(yuca::Document(1)); source.indexDocument(yuca::Document(2));
    auto bytes = source.serialize();
    // Empty records are 56 bytes. Header+implicit group+document count is 64.
    put64(bytes, 64 + 56, 1); repair(bytes);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(bytes), yuca::SnapshotError);
    REQUIRE(source.getDocument(2).getId() == 2);
}

TEST_CASE("Snapshot structural validation rejects duplicate names empty groups and wrong key hashes", "[snapshot]") {
    auto header = yuca::Indexer().serialize(); header.resize(40);
    SECTION("duplicate properties") {
        appendText(header, ":keyword"); append64(header, 1); append64(header, 7); append64(header, 0);
        append64(header, 2);
        for (unsigned i = 0; i < 2; ++i) { appendText(header, "flag"); append64(header, 1); }
        for (unsigned i = 0; i < 4; ++i) append64(header, 0);
    }
    SECTION("empty key group") {
        appendText(header, ":keyword"); append64(header, 1); append64(header, 7); append64(header, 1);
        appendText(header, ":keyword"); append64(header, 0);
        for (unsigned i = 0; i < 5; ++i) append64(header, 0);
    }
    SECTION("wrong key hash") {
        appendText(header, ":keyword"); append64(header, 1); append64(header, 7); append64(header, 1);
        appendText(header, ":keyword"); append64(header, 1); appendText(header, "hello");
        append64(header, static_cast<std::uint64_t>(yuca::StringKey("hello", ":keyword").getId()) ^ 1);
        for (unsigned i = 0; i < 5; ++i) append64(header, 0);
    }
    SECTION("duplicate groups") {
        appendText(header, ":keyword"); append64(header, 1); append64(header, 7); append64(header, 2);
        for (unsigned i = 0; i < 2; ++i) {
            appendText(header, ":keyword"); append64(header, 1); appendText(header, "hello");
            append64(header, static_cast<std::uint64_t>(yuca::StringKey("hello", ":keyword").getId()));
        }
        for (unsigned i = 0; i < 5; ++i) append64(header, 0);
    }
    SECTION("integer out of native range") {
        appendText(header, ":keyword"); append64(header, 1); append64(header, 7); append64(header, 0);
        append64(header, 0); append64(header, 0);
        append64(header, 1); appendText(header, "integer"); append64(header, std::numeric_limits<std::uint64_t>::max() / 2);
        append64(header, 0); append64(header, 0);
    }
    repair(header);
    REQUIRE_THROWS_AS(yuca::Indexer::deserialize(header), yuca::SnapshotError);
}

#ifndef _WIN32
TEST_CASE("Snapshot files replace checkpoints and preserve the last good file on failure", "[snapshot][snapshot-file]") {
    struct Directory {
        std::string path;
        Directory() {
            char name[] = "snapshot-test-XXXXXX";
            const auto result = mkdtemp(name);
            if (!result) throw std::runtime_error("cannot create test directory");
            path = result;
        }
        ~Directory() { std::remove((path + "/state.yuca").c_str()); rmdir((path + "/directory").c_str()); rmdir(path.c_str()); }
    } directory;
    const auto path = directory.path + "/state.yuca";
    auto source = fixture(); source.save(path);
    REQUIRE(yuca::Indexer::load(path).serialize() == source.serialize());
    source.removeDocument(7); source.save(path);
    REQUIRE(yuca::Indexer::load(path).search("hello").size() == 3);
    yuca::SnapshotLimits limits; limits.max_bytes = 1;
    REQUIRE_THROWS_AS(yuca::Indexer::load(path, limits), yuca::SnapshotError);
    REQUIRE_THROWS_AS(source.save(directory.path + "/absent/state.yuca"), yuca::SnapshotError);
    REQUIRE(mkdir((directory.path + "/directory").c_str(), 0700) == 0);
    REQUIRE_THROWS_AS(source.save(directory.path + "/directory"), yuca::SnapshotError); // rename failure, not creation failure
    REQUIRE_THROWS_AS(yuca::Indexer::load(directory.path + "/absent.yuca"), yuca::SnapshotError);
    REQUIRE_THROWS_AS(source.save(std::string("bad\0path", 8)), yuca::SnapshotError);
    REQUIRE_THROWS_AS(yuca::Indexer::load(""), yuca::SnapshotError);
    REQUIRE(yuca::Indexer::load(path).serialize() == source.serialize());
    struct NonstandardKey : yuca::StringKey {
        NonstandardKey() : StringKey("bad", ":keyword") { id ^= 1; }
    };
    yuca::Document bad_document(8); bad_document.addKey(std::make_shared<NonstandardKey>());
    yuca::Indexer bad_index; bad_index.indexDocument(bad_document);
    REQUIRE_THROWS_AS(bad_index.save(path), yuca::SnapshotError);
    REQUIRE(yuca::Indexer::load(path).serialize() == source.serialize()); // Encode failure never opens the old checkpoint.
    { std::ofstream truncated(path.c_str(), std::ios::binary | std::ios::trunc); truncated << "YUCA"; }
    REQUIRE_THROWS_AS(yuca::Indexer::load(path), yuca::SnapshotError);
}
#endif
