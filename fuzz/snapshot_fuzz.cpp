#include "yuca/yuca.hpp"
#include "../src/yuca/snapshot_checksum.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace {
    void check(bool value) { if (!value) std::abort(); }
    void tryDecode(const unsigned char *data, std::size_t size) {
        yuca::SnapshotLimits limits;
        limits.max_bytes = 65536; limits.max_documents = 256;
        limits.max_keys = 2048; limits.max_properties = 2048; limits.max_string_bytes = 4096;
        limits.max_decoded_string_bytes = 65536;
        try {
            auto index = yuca::Indexer::deserialize(data, size, limits);
            const auto canonical = index.serialize();
            auto loaded = yuca::Indexer::deserialize(canonical, limits);
            check(loaded.serialize() == canonical);
        } catch (const yuca::SnapshotError &) { /* Rejection is the parser contract. */ }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
    if (size > 65536) return 0;
    tryDecode(data, size);
    yuca::Indexer index(":title");
    for (std::size_t i = 0; i < size && i < 16; ++i) {
        yuca::Document document(static_cast<long>(i));
        document.addKey(yuca::StringKey(std::to_string(data[i] % 8), ":title"));
        document.stringProperty("payload", std::string(reinterpret_cast<const char *>(data), size < 128 ? size : 128));
        document.intProperty("number", data[i]);
        index.indexDocument(document);
    }
    const auto bytes = index.serialize();
    auto loaded = yuca::Indexer::deserialize(bytes);
    check(loaded.serialize() == bytes);
    for (unsigned i = 0; i < 8; ++i) {
        const auto before = index.search(std::to_string(i));
        const auto after = loaded.search(std::to_string(i));
        check(before.size() == after.size());
        for (unsigned long j = 0; j < before.size(); ++j) {
            check(before.get(static_cast<long>(j)).document_sp->getId() == after.get(static_cast<long>(j)).document_sp->getId());
            check(before.get(static_cast<long>(j)).score == after.get(static_cast<long>(j)).score);
        }
    }
    if (size != 0) {
        // Repair the checksum after payload mutation so fuzzing reaches structural
        // checks instead of stopping at the integrity guard on every execution.
        auto mutated = bytes;
        for (std::size_t i = 0; i < size; ++i) mutated[40 + i % (mutated.size() - 40)] ^= data[i];
        const auto hash = yuca::detail::snapshotChecksum(mutated.data() + 40, mutated.size() - 40);
        for (unsigned i = 0; i < 8; ++i) mutated[24 + i] = static_cast<unsigned char>(hash >> (i * 8));
        tryDecode(mutated.data(), mutated.size());
    }
    return 0;
}
