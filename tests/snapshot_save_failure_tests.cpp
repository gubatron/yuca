// POSIX-only diagnostic executable: never link its allocator into the library.
#include "yuca/yuca.hpp"
#include "support/allocation_tracker.hpp"
#include <dirent.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    std::string directory, path;
    try {
        if (argc != 2) throw std::runtime_error("expected a build-directory temporary prefix");
        std::string pattern = std::string(argv[1]) + "XXXXXX";
        std::vector<char> name(pattern.begin(), pattern.end()); name.push_back(0);
        if (!mkdtemp(name.data())) throw std::runtime_error("cannot create snapshot failure test directory");
        directory = name.data(); path = directory + "/state.yuca";
        yuca::Indexer old;
        yuca::Document document(1); document.addKey(yuca::StringKey("old", ":keyword")); old.indexDocument(document);
        yuca::Indexer updated(old);
        yuca::Document next(2); next.addKey(yuca::StringKey("new", ":keyword")); updated.indexDocument(next);
        const auto old_bytes = old.serialize(), new_bytes = updated.serialize();
        bool succeeded = false;
        for (std::size_t point = 0; point < 4096; ++point) {
            old.save(path);
            const auto baseline = yuca_test::allocationSnapshot().live_bytes;
            bool failed = false;
            yuca_test::failAllocationAfter(point);
            try { updated.save(path); }
            catch (const std::bad_alloc &) { failed = true; }
            yuca_test::disableAllocationFailure();
            if (yuca_test::allocationSnapshot().live_bytes != baseline) throw std::runtime_error("save leaked after failure");
            if (yuca::Indexer::load(path).serialize() != (failed ? old_bytes : new_bytes))
                throw std::runtime_error("failed save destroyed last good checkpoint");
            DIR *entries = opendir(directory.c_str());
            if (!entries) throw std::runtime_error("cannot inspect temporary cleanup");
            bool clean = true;
            while (const auto *entry = readdir(entries)) {
                const std::string value(entry->d_name);
                if (value != "." && value != ".." && value != "state.yuca") clean = false;
            }
            closedir(entries);
            if (!clean) throw std::runtime_error("save left temporary files after failure");
            if (!failed) { std::cout << "snapshot save: " << point << " allocation failures preserve old bytes, then success\n"; succeeded = true; break; }
        }
        if (!succeeded) throw std::runtime_error("save failure sweep exceeded budget");
        std::remove(path.c_str()); rmdir(directory.c_str());
    } catch (const std::exception &error) {
        yuca_test::disableAllocationFailure();
        if (!path.empty()) std::remove(path.c_str());
        if (!directory.empty()) rmdir(directory.c_str());
        std::cerr << error.what() << '\n'; return 1;
    }
}
