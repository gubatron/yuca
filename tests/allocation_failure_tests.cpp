#include "yuca/yuca.hpp"
#include "support/allocation_tracker.hpp"
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>

namespace {
    void check(bool valid, const char *message) {
        if (!valid) {
            throw std::runtime_error(message);
        }
    }

    template<class Prepare, class Operation, class VerifyFailure, class VerifySuccess>
    void exhaust(const char *name, Prepare prepare, Operation operation,
                 VerifyFailure verify_failure, VerifySuccess verify_success) {
        for (std::size_t point = 0; point < 4096; ++point) {
            const auto live_before = yuca_test::allocationSnapshot().live_bytes;
            auto state = prepare();
            bool failed = false;
            yuca_test::failAllocationAfter(point);
            try {
                operation(*state);
            } catch (const std::bad_alloc &) {
                failed = true;
            }
            yuca_test::disableAllocationFailure();
            if (failed) {
                try { verify_failure(*state); }
                catch (...) {
                    std::cerr << name << ": invariant failed at allocation " << point << '\n';
                    throw;
                }
            } else {
                verify_success(*state);
                state.reset();
                check(yuca_test::allocationSnapshot().live_bytes == live_before,
                      "successful mutation leaked tracked allocation bytes");
                std::cout << name << ": verified " << point << " failure points and success\n";
                return;
            }
            state.reset();
            check(yuca_test::allocationSnapshot().live_bytes == live_before,
                  "failed mutation leaked tracked allocation bytes");
        }
        throw std::runtime_error("allocation failure sweep exceeded its bounded budget");
    }

    yuca::Document oldDocument() {
        yuca::Document document(42);
        document.addKey(yuca::StringKey("old", ":keyword"));
        document.addKey(yuca::StringKey("txt", ":extension"));
        return document;
    }

    yuca::Document newDocument() {
        yuca::Document document(42);
        document.addKey(yuca::StringKey("new", ":keyword"));
        document.addKey(yuca::StringKey("pdf", ":kind"));
        return document;
    }
}

int main(int argc, char **argv) {
    try {
        const std::string selected = argc == 2 ? argv[1] : "all";
        check(selected == "all" || selected == "map" || selected == "posting" || selected == "replace" ||
              selected == "insert" || selected == "remove" || selected == "copy", "unknown failure scenario");
        if (selected == "all" || selected == "map") {
            auto map = yuca::utils::Map<int, std::string>("");
            map.put(1, "old");
            std::string replacement(100, 'x');
            yuca_test::failAllocationAfter(0);
            try { map.put(1, std::move(replacement)); }
            catch (const std::bad_alloc &) {}
            yuca_test::disableAllocationFailure();
            check(map.get(1) == "old" || map.get(1) == std::string(100, 'x'),
                  "map replacement lost the old mapping on failure");
        }
        if (selected == "all" || selected == "posting") {
            const auto key = std::make_shared<yuca::StringKey>("key", ":keyword");
            const auto doc = std::make_shared<yuca::Document>(1);
            exhaust("posting insertion", [] { return std::make_shared<yuca::ReverseIndex>(); },
                [&](yuca::ReverseIndex &index) { index.putDocument(key, doc); },
                [&](yuca::ReverseIndex &index) {
                    check(index.getKeyCount() == 0 && index.keyCacheGet(key) == nullptr,
                          "failed posting insertion published a partial key/cache");
                }, [&](yuca::ReverseIndex &index) {
                    check(index.getDocuments(key).size() == 1, "successful posting insertion missing");
                });
        }
        const auto replacement = newDocument();
        auto prepare = [] {
            auto index = std::make_shared<yuca::Indexer>();
            index->indexDocument(oldDocument());
            return index;
        };
        auto unchanged = [](yuca::Indexer &index) {
            check(index.search("old :extension txt").size() == 1 && index.search("new").isEmpty(),
                  "failed mutation changed previous postings");
            check(index.getDocument(42).getGroupKeys(":keyword").contains(yuca::StringKey("old", ":keyword")),
                  "failed mutation changed cached document");
        };
        if (selected == "all" || selected == "replace") {
            exhaust("document replacement", prepare,
                [&](yuca::Indexer &index) { index.indexDocument(replacement); }, unchanged,
                [](yuca::Indexer &index) {
                    check(index.search("old").isEmpty() && index.search("new :kind pdf").size() == 1,
                          "successful replacement has stale/missing postings");
                });
        }
        if (selected == "all" || selected == "insert") {
            yuca::Document candidate(7);
            candidate.addKey(yuca::StringKey("new", ":keyword"));
            candidate.addKey(yuca::StringKey("pdf", ":kind"));
            exhaust("document insertion", prepare,
                [&](yuca::Indexer &index) { index.indexDocument(candidate); },
                [&](yuca::Indexer &index) {
                    unchanged(index);
                    check(index.getDocument(7) == yuca::Document::NULL_DOCUMENT,
                          "failed insertion published cache entry");
                }, [](yuca::Indexer &index) {
                    check(index.search("new :kind pdf").size() == 1, "successful insertion missing");
                });
        }
        if (selected == "all" || selected == "remove") {
            exhaust("document removal", prepare,
                [](yuca::Indexer &index) { check(index.removeDocument(42), "document was not removed"); },
                unchanged, [](yuca::Indexer &index) {
                    check(index.search("old").isEmpty() && index.getDocument(42) == yuca::Document::NULL_DOCUMENT,
                          "successful removal left cached data/postings");
                });
        }
        if (selected == "all" || selected == "copy") {
            exhaust("index copy", prepare, [](yuca::Indexer &index) { yuca::Indexer copy(index); },
                    unchanged, unchanged);
        }
        return 0;
    } catch (const std::exception &error) {
        yuca_test::disableAllocationFailure();
        std::cerr << error.what() << '\n';
        return 1;
    }
}
