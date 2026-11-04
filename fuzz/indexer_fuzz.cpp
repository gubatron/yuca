#include "yuca/yuca.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
    struct ReferenceDocument {
        unsigned term;
        unsigned second_term;
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
                unsigned keyword, unsigned second_keyword, unsigned extension, bool grouped, unsigned long limit) {
        const std::string query = term(keyword) + " " + term(second_keyword) +
            (grouped ? " :extension " + term(extension) : "");
        std::vector<std::pair<long, unsigned>> expected;
        for (const auto &entry : reference) {
            const auto matches = [&](unsigned value) {
                return entry.second.term == value || entry.second.second_term == value;
            };
            const unsigned score = static_cast<unsigned>(matches(keyword)) +
                                   static_cast<unsigned>(matches(second_keyword));
            if (score != 0 && (!grouped || entry.second.extension == extension)) {
                expected.emplace_back(entry.first, score + (grouped ? 1U : 0U));
            }
        }
        std::sort(expected.begin(), expected.end(), [](const std::pair<long, unsigned> &a,
                                                     const std::pair<long, unsigned> &b) {
            return a.second == b.second ? a.first < b.first : a.second > b.second;
        });
        const auto results = indexer.search(query, "", limit);
        check(results.size() == (limit == 0 ? expected.size() : std::min(limit, expected.size())));
        std::set<long> seen;
        std::size_t position = 0;
        for (const auto &result : results.getStdVector()) {
            const long id = result.document_sp->getId();
            check(seen.insert(id).second);
            check(id == expected[position].first);
            check(result.score == expected[position].second);
            ++position;
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
        const unsigned action = data[offset] % 6;
        const long id = static_cast<long>(data[offset + 1] % 16) - 8;
        const unsigned keyword = data[offset + 2] % 8;
        const unsigned extension = data[offset + 3] % 2;
        const unsigned second_keyword = (data[offset + 3] / 2) % 8;
        if (action <= 1) {
            auto doc = std::make_shared<yuca::Document>(id);
            doc->addKey(yuca::StringKey(term(keyword), ":keyword"));
            doc->addKey(yuca::StringKey(term(second_keyword), ":keyword"));
            doc->addKey(yuca::StringKey(term(extension), ":extension"));
            indexer.indexDocument(doc);
            reference[id] = ReferenceDocument{keyword, second_keyword, extension};
        } else if (action == 2) {
            const bool existed = reference.erase(id) != 0;
            check(indexer.removeDocument(id) == existed);
        } else if (action == 3) {
            // Removal must use stored keys, not keys on this equivalent empty object.
            indexer.removeDocument(std::make_shared<yuca::Document>(id));
            reference.erase(id);
        } else if (action == 4 && data[offset + 1] == 0) {
            indexer.clear();
            reference.clear();
        } else if (action == 5) {
            yuca::Indexer copied(indexer);
            verify(copied, reference, keyword, second_keyword, extension, true, 0);
            copied.clear(); // Must not change the original's postings or cached documents.
        }
        verify(indexer, reference, keyword, second_keyword, extension, false, 0);
        verify(indexer, reference, keyword, second_keyword, extension, true, 0);
        verify(indexer, reference, keyword, second_keyword, extension, false,
               data[offset] % 2 ? 1 : std::numeric_limits<unsigned long>::max());
    }
    if (size != 0) {
        // Also exercise the query parser with arbitrary bytes, including embedded NUL.
        indexer.search(std::string(reinterpret_cast<const char *>(data), size), "", 5);
    }
    return 0;
}
