#include "yuca/yuca.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <set>
#include <string>

namespace {
    struct ReferenceDocument {
        unsigned term;
        unsigned extension;
    };

    void check(bool valid) {
        if (!valid) {
            std::abort();
        }
    }

    std::string term(unsigned value) {
        return "term" + std::to_string(value);
    }

    void verify(yuca::Indexer &indexer, const std::map<long, ReferenceDocument> &reference,
                unsigned keyword, unsigned extension, bool grouped, unsigned long limit) {
        const std::string query = term(keyword) + (grouped ? " :extension " + term(extension) : "");
        std::set<long> expected;
        for (const auto &entry : reference) {
            if (entry.second.term == keyword && (!grouped || entry.second.extension == extension)) {
                expected.insert(entry.first);
            }
        }
        const auto results = indexer.search(query, "", limit);
        check(results.size() == (limit == 0 ? expected.size() : std::min(limit, expected.size())));
        std::set<long> seen;
        double previous_score = std::numeric_limits<double>::infinity();
        for (const auto &result : results.getStdVector()) {
            const long id = result.document_sp->getId();
            check(expected.count(id) == 1);
            check(seen.insert(id).second);
            check(result.score <= previous_score);
            check(result.score == (grouped ? 2.0 : 1.0));
            previous_score = result.score;
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
    if (size > 512) {
        return 0;
    }
    yuca::Indexer indexer;
    std::map<long, ReferenceDocument> reference;
    for (std::size_t offset = 0; offset + 3 < size; offset += 4) {
        const unsigned action = data[offset] % 5;
        const long id = static_cast<long>(data[offset + 1] % 16) - 8;
        const unsigned keyword = data[offset + 2] % 8;
        const unsigned extension = data[offset + 3] % 2;
        if (action <= 1) {
            auto doc = std::make_shared<yuca::Document>(id);
            doc->addKey(yuca::StringKey(term(keyword), ":keyword"));
            doc->addKey(yuca::StringKey(term(extension), ":extension"));
            indexer.indexDocument(doc);
            reference[id] = ReferenceDocument{keyword, extension};
        } else if (action == 2) {
            const bool existed = reference.erase(id) != 0;
            check(indexer.removeDocument(id) == existed);
        } else if (action == 3) {
            // Removal must use stored keys, not keys on this equivalent empty object.
            indexer.removeDocument(std::make_shared<yuca::Document>(id));
            reference.erase(id);
        } else if (data[offset + 1] == 0) {
            indexer.clear();
            reference.clear();
        }
        verify(indexer, reference, keyword, extension, false, 0);
        verify(indexer, reference, keyword, extension, true, 0);
        verify(indexer, reference, keyword, extension, false, 1);
    }
    if (size != 0) {
        // Also exercise the query parser with arbitrary bytes, including embedded NUL.
        indexer.search(std::string(reinterpret_cast<const char *>(data), size), "", 5);
    }
    return 0;
}
