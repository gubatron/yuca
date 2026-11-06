#include "yuca/yuca.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
    try {
        if (argc != 2) throw std::invalid_argument("usage: persistence-example checkpoint.yuca (replaces that file)");
        yuca::Indexer index(":title");
        yuca::Document first(7);
        first.addKey(yuca::StringKey("hello", ":title"));
        first.stringProperty("path", "/documents/first.txt");
        index.indexDocument(first);
        index.save(argv[1]);

        auto resumed = yuca::Indexer::load(argv[1]);
        if (resumed.search("hello").size() != 1 || resumed.getDocument(7).stringProperty("path") != "/documents/first.txt")
            throw std::runtime_error("checkpoint validation failed");
        yuca::Document next(8);
        next.addKey(yuca::StringKey("world", ":title"));
        resumed.indexDocument(next); // Incremental work after loading.
        resumed.save(argv[1]);
        auto verified = yuca::Indexer::load(argv[1]);
        if (verified.search("hello world").size() != 2) throw std::runtime_error("incremental checkpoint failed");
        std::cout << "2 documents restored; incremental checkpoint verified\n";
    } catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
