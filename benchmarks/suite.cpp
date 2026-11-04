#include "corpus.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>

namespace {
    typedef std::chrono::steady_clock Clock;

    unsigned parse(const char *text, bool allow_zero = false) {
        std::size_t consumed = 0;
        const auto value = std::stoul(text, &consumed);
        if (consumed != std::string(text).size() || (!allow_zero && value == 0) || value > 100000) {
            throw std::invalid_argument("argument outside supported range");
        }
        return static_cast<unsigned>(value);
    }

    void require(bool valid) {
        if (!valid) {
            throw std::runtime_error("benchmark result validation failed");
        }
    }

    std::uint64_t fingerprint(const yuca::utils::List<yuca::SearchResult> &results, unsigned long expected) {
        require(results.size() == expected);
        std::set<long> seen;
        std::uint64_t hash = 14695981039346656037ULL;
        double previous_score = std::numeric_limits<double>::infinity();
        long previous_id = std::numeric_limits<long>::min();
        for (const auto &result : results.getStdVector()) {
            const auto id = result.document_sp->getId();
            require(std::isfinite(result.score) && result.score >= 0 && seen.insert(id).second);
            require(result.score < previous_score || (result.score == previous_score && id > previous_id));
            hash = (hash ^ static_cast<std::uint64_t>(id)) * 1099511628211ULL;
            std::uint64_t score_bits = 0;
            static_assert(sizeof(score_bits) == sizeof(result.score), "benchmark requires a 64-bit double");
            std::memcpy(&score_bits, &result.score, sizeof(score_bits));
            hash = (hash ^ score_bits) * 1099511628211ULL;
            previous_score = result.score;
            previous_id = id;
        }
        return hash;
    }

    template<class Operation>
    double elapsed(Operation operation) {
        const auto start = Clock::now();
        operation();
        return std::chrono::duration<double, std::nano>(Clock::now() - start).count();
    }
}

int main(int argc, char **argv) {
    try {
        if (argc != 5) {
            throw std::invalid_argument("usage: yuca_benchmark_suite workload documents iterations top_k");
        }
        const std::string workload(argv[1]);
        const unsigned count = parse(argv[2]);
        const unsigned iterations = parse(argv[3]);
        const unsigned top_k = parse(argv[4], true);
        const bool mutation = workload == "index" || workload == "replace" || workload == "remove" || workload == "clear";
        const bool query = workload == "search_rare" || workload == "search_common" ||
                           workload == "search_grouped" || workload == "search_distance" || workload == "search_empty";
        require(mutation || query || workload == "copy");
        const auto documents = yuca_bench::makeCorpus(count);
        std::vector<yuca::Document> replacements;
        if (workload == "replace") {
            replacements.reserve(count);
            for (unsigned id = 0; id < count; ++id) {
                yuca::Document doc(static_cast<long>(id));
                doc.addKey(yuca::StringKey("replacement", ":keyword"));
                replacements.push_back(doc);
            }
        }
        yuca::Indexer prepared;
        if (!mutation) {
            yuca_bench::populate(prepared, documents);
        }
        double total_ns = 0;
        std::uint64_t checksum = 14695981039346656037ULL;
        // Each fresh sample process performs the same operation once untimed
        // before recording iterations, rather than relying on another process's cache.
        for (unsigned pass = 0; pass <= iterations; ++pass) {
            const unsigned iteration = pass == 0 ? 0 : pass - 1;
            double sample_ns = 0;
            std::uint64_t iteration_checksum = 0;
            if (mutation) {
                yuca::Indexer index;
                if (workload != "index") {
                    yuca_bench::populate(index, documents); // setup outside timer
                }
                unsigned removed = 0;
                sample_ns = elapsed([&] {
                    if (workload == "clear") {
                        index.clear();
                    } else if (workload == "remove") {
                        for (unsigned id = 0; id < count; ++id) {
                            removed += index.removeDocument(static_cast<long>(id)) ? 1U : 0U;
                        }
                    } else {
                        yuca_bench::populate(index, workload == "replace" ? replacements : documents);
                    }
                });
                if (workload == "remove") {
                    require(removed == count);
                }
                const auto results = index.search(workload == "replace" ? "replacement" : "common");
                iteration_checksum = fingerprint(results, workload == "remove" || workload == "clear" ? 0 : count);
                if (workload == "replace") {
                    require(index.search("common").isEmpty());
                }
            } else if (workload == "copy") {
                std::unique_ptr<yuca::Indexer> clone;
                sample_ns = elapsed([&] { clone.reset(new yuca::Indexer(prepared)); });
                iteration_checksum = fingerprint(clone->search("common"), count);
            } else {
                const auto token = iteration % 100;
                const std::string text = workload == "search_rare" ? "term" + std::to_string(token) :
                    workload == "search_grouped" ? "common :extension txt" :
                    workload == "search_empty" ? "not-indexed" : "common";
                const std::string property = workload == "search_distance" ? "title" : "";
                unsigned long expected = workload == "search_rare" ? (count > token ? (count - 1UL - token) / 100 + 1 : 0) :
                    workload == "search_grouped" ? count / 2UL : workload == "search_empty" ? 0 : count;
                if (top_k != 0) {
                    expected = std::min<unsigned long>(expected, top_k);
                }
                yuca::utils::List<yuca::SearchResult> results;
                sample_ns = elapsed([&] { results = prepared.search(text, property, top_k); });
                iteration_checksum = fingerprint(results, expected); // validation/teardown outside timer
            }
            if (pass != 0) {
                total_ns += sample_ns;
                checksum = (checksum ^ iteration_checksum) * 1099511628211ULL;
            }
        }
        const unsigned long long operations = static_cast<unsigned long long>(iterations) *
            ((mutation && workload != "clear") ? count : 1U);
        std::cout << std::setprecision(17) << "{\"schema\":1,\"workload\":\"" << workload
                  << "\",\"documents\":" << count << ",\"iterations\":" << iterations << ",\"top_k\":" << top_k
                  << ",\"operations\":" << operations << ",\"elapsed_ns\":" << total_ns
                  << ",\"ns_per_operation\":" << total_ns / static_cast<double>(operations)
                  << ",\"checksum\":" << checksum << "}\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
