#include "yuca/yuca.hpp"
#include "corpus.hpp"
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    volatile unsigned long checksum = 0;
    typedef std::chrono::steady_clock Clock;

    template<class Work>
    void measure(const char *name, unsigned samples, unsigned operations, Work work) {
        work(); // warmup, outside the samples
        for (unsigned sample = 0; sample < samples; ++sample) {
            const auto start = Clock::now();
            const unsigned long result = work();
            const auto stop = Clock::now();
            checksum ^= result;
            const double ns = std::chrono::duration<double, std::nano>(stop - start).count();
            std::cout << name << ',' << sample << ',' << std::fixed << std::setprecision(2)
                      << ns / operations << ',' << result << '\n';
        }
    }

    unsigned parse(const char *text, unsigned maximum) {
        std::size_t consumed = 0;
        const unsigned long value = std::stoul(text, &consumed);
        if (consumed != std::string(text).size() || value == 0 || value > maximum) {
            throw std::invalid_argument("argument outside supported range");
        }
        return static_cast<unsigned>(value);
    }
}

int main(int argc, char **argv) {
    try {
        if (argc > 3) {
            throw std::invalid_argument("usage: yuca_benchmarks [samples=7] [documents=1000]");
        }
        const unsigned samples = argc > 1 ? parse(argv[1], 100) : 7;
        const unsigned count = argc > 2 ? parse(argv[2], 100000) : 1000;
        const auto documents = yuca_bench::makeCorpus(count);
        std::cout << "# compiler="
#ifdef _MSC_VER
                  << "MSVC " << _MSC_VER
#else
                  << __VERSION__
#endif
                  << " cplusplus=" << __cplusplus
                  << " documents=" << count << " samples=" << samples << '\n';
        std::cout << "workload,sample,ns_per_operation,checksum\n";
        measure("index_document", samples, count, [&]() {
            yuca::Indexer indexer;
            for (const auto &doc : documents) {
                indexer.indexDocument(doc);
            }
            return indexer.search("common").size(); // includes validation/search and teardown
        });
        yuca::Indexer indexer;
        for (const auto &doc : documents) {
            indexer.indexDocument(doc);
        }
        const unsigned searches = 20;
        measure("search_grouped", samples, searches, [&]() {
            unsigned long sum = 0;
            for (unsigned i = 0; i < searches; ++i) {
                sum += indexer.search("common :extension txt").size();
            }
            return sum;
        });
        measure("search_top10", samples, searches, [&]() {
            unsigned long sum = 0;
            for (unsigned i = 0; i < searches; ++i) {
                sum += indexer.search("common", "", 10).size();
            }
            return sum;
        });
        measure("search_levenshtein", samples, searches, [&]() {
            unsigned long sum = 0;
            for (unsigned i = 0; i < searches; ++i) {
                sum += indexer.search("common", "title", 10).size();
            }
            return sum;
        });
        measure("remove_document", samples, count, [&]() {
            yuca::Indexer copy; // rebuilding is intentionally part of this lifecycle workload
            for (const auto &doc : documents) {
                copy.indexDocument(doc);
            }
            unsigned long removed = 0;
            for (unsigned i = 0; i < count; ++i) {
                removed += copy.removeDocument(static_cast<long>(i)) ? 1 : 0;
            }
            return removed;
        });
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
