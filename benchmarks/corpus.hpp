#ifndef YUCA_BENCHMARK_CORPUS_HPP
#define YUCA_BENCHMARK_CORPUS_HPP

#include "yuca/yuca.hpp"
#include <string>
#include <vector>

namespace yuca_bench {
    inline std::vector<yuca::Document> makeCorpus(unsigned count) {
        std::vector<yuca::Document> documents;
        documents.reserve(count);
        for (unsigned i = 0; i < count; ++i) {
            yuca::Document doc(static_cast<long>(i));
            doc.addKey(yuca::StringKey("common", ":keyword"));
            doc.addKey(yuca::StringKey("term" + std::to_string(i % 100), ":keyword"));
            doc.addKey(yuca::StringKey(i % 2 ? "txt" : "pdf", ":extension"));
            doc.stringProperty("title", "common title " + std::to_string(i));
            documents.push_back(doc);
        }
        return documents;
    }

    inline void populate(yuca::Indexer &index, const std::vector<yuca::Document> &documents) {
        for (const auto &doc : documents) {
            index.indexDocument(doc);
        }
    }
}

#endif
