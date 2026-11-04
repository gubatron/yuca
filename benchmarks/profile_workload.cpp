#include "corpus.hpp"
#ifdef YUCA_ALLOCATION_METRICS
#include "../tests/support/allocation_tracker.hpp"
#endif
#include <iostream>
#include <stdexcept>
#include <string>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif

namespace {
    unsigned parse(const char *text, unsigned maximum) {
        std::size_t consumed = 0;
        const auto value = std::stoul(text, &consumed);
        if (consumed != std::string(text).size() || value == 0 || value > maximum) {
            throw std::invalid_argument("argument outside supported range");
        }
        return static_cast<unsigned>(value);
    }

    unsigned long long peakRssBytes() {
#if defined(__APPLE__) || defined(__linux__)
        struct rusage usage {};
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
#if defined(__APPLE__)
            return static_cast<unsigned long long>(usage.ru_maxrss);
#else
            return static_cast<unsigned long long>(usage.ru_maxrss) * 1024;
#endif
        }
#endif
        return 0; // Unsupported metric, not a claim of zero process memory.
    }
}

int main(int argc, char **argv) {
    try {
        if (argc < 2 || argc > 4) {
            throw std::invalid_argument("usage: yuca_workload workload [documents=1000] [iterations=100]");
        }
        const std::string workload(argv[1]);
        const unsigned count = argc > 2 ? parse(argv[2], 100000) : 1000;
        const unsigned iterations = argc > 3 ? parse(argv[3], 100000) : 100;
        if (workload != "index" && workload != "search" && workload != "search_grouped" &&
            workload != "search_top10" && workload != "search_distance") {
            throw std::invalid_argument("unknown workload");
        }
        const auto documents = yuca_bench::makeCorpus(count);
        yuca::Indexer index;
        if (workload != "index") {
            yuca_bench::populate(index, documents);
        }
#ifdef YUCA_ALLOCATION_METRICS
        yuca_test::resetAllocationPeak();
        const auto before = yuca_test::allocationSnapshot();
#endif
        unsigned long checksum = 0;
        if (workload == "index") {
            yuca_bench::populate(index, documents);
            checksum = count;
        } else {
            const std::string query = workload == "search_grouped" ? "common :extension txt" :
                workload == "search" ? "term0" : "common";
            const std::string property = workload == "search_distance" ? "title" : "";
            const unsigned long limit = workload == "search_top10" || workload == "search_distance" ? 10 : 0;
            for (unsigned i = 0; i < iterations; ++i) {
                checksum += index.search(query, property, limit).size();
            }
        }
#ifdef YUCA_ALLOCATION_METRICS
        const auto after = yuca_test::allocationSnapshot();
#endif
        const unsigned long expected = workload == "index" ? count :
            iterations * (workload == "search" ? (count + 99UL) / 100 :
                workload == "search_grouped" ? count / 2UL : std::min<unsigned long>(10, count));
        if (checksum != expected) {
            throw std::runtime_error("workload output validation failed");
        }
        std::cout << "{\"workload\":\"" << workload << "\",\"documents\":" << count
                  << ",\"iterations\":" << (workload == "index" ? 1 : iterations)
                  << ",\"checksum\":" << checksum << ",\"peak_process_rss_bytes\":" << peakRssBytes();
#ifdef YUCA_ALLOCATION_METRICS
        std::cout << ",\"new_calls\":" << after.allocations - before.allocations
                  << ",\"new_requested_bytes\":" << after.allocated_bytes - before.allocated_bytes
                  << ",\"live_new_bytes_before\":" << before.live_bytes
                  << ",\"live_new_bytes_after\":" << after.live_bytes
                  << ",\"peak_extra_live_new_bytes\":" << after.peak_bytes - before.live_bytes;
#endif
        std::cout << "}\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
