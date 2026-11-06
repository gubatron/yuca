#include "corpus.hpp"
#include "../src/yuca/snapshot_detail.h"
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
    template<class Work> double milliseconds(Work work) {
        const auto start = std::chrono::steady_clock::now();
        work();
        return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    }
}

int main(int argc, char **argv) {
    try {
        std::size_t used = 0;
        const std::string argument = argc > 1 ? argv[1] : "10000";
        const auto parsed = std::stoul(argument, &used);
        if (used != argument.size() || parsed == 0 || parsed > 1000000) throw std::invalid_argument("documents must be 1..1000000");
        const auto count = static_cast<unsigned>(parsed);
        const std::string path = argc > 2 ? argv[2] : "";
        yuca::Indexer index;
        for (auto doc : yuca_bench::makeCorpus(count)) {
            doc.boolProperty("seen", true); doc.byteProperty("kind", 7);
            doc.intProperty("revision", 1); doc.longProperty("timestamp", 123456789);
            index.indexDocument(doc);
        }
        const auto expected = index.serialize();
        std::cout << "documents,sample,bytes,encode_ms,bulk_decode_ms,replay_decode_ms,save_ms,load_ms\n" << std::setprecision(9);
        for (unsigned sample = 0; sample < 8; ++sample) {
            std::vector<unsigned char> bytes;
            const auto encode_ms = milliseconds([&] { bytes = index.serialize(); });
            std::unique_ptr<yuca::Indexer> bulk, replay;
            double bulk_ms = 0, replay_ms = 0;
            const auto run_bulk = [&] { bulk_ms = milliseconds([&] { bulk.reset(new yuca::Indexer(yuca::Indexer::deserialize(bytes))); }); };
            const auto run_replay = [&] { replay_ms = milliseconds([&] { replay.reset(new yuca::Indexer(yuca::detail::deserializeByReplay(bytes))); }); };
            if (sample % 2 == 0) { run_bulk(); run_replay(); } else { run_replay(); run_bulk(); }
            if (bytes != expected || bulk->serialize() != bytes || replay->serialize() != bytes ||
                bulk->search("term0").size() != (count + 99UL) / 100) throw std::runtime_error("snapshot benchmark validation failed");
            bulk.reset(); replay.reset(); // Destruction is outside decode timers.
            double save_ms = 0, load_ms = 0;
            if (!path.empty()) {
                save_ms = milliseconds([&] { index.save(path); });
                load_ms = milliseconds([&] { bulk.reset(new yuca::Indexer(yuca::Indexer::load(path))); });
                if (bulk->serialize() != bytes) throw std::runtime_error("file snapshot validation failed");
            }
            if (sample != 0) std::cout << count << ',' << sample << ',' << bytes.size() << ',' << encode_ms << ','
                << bulk_ms << ',' << replay_ms << ',' << save_ms << ',' << load_ms << '\n';
        }
        // A caller-supplied benchmark output is disposable, not a backup path.
        if (!path.empty() && std::remove(path.c_str()) != 0) throw std::runtime_error("cannot remove benchmark snapshot");
    } catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
