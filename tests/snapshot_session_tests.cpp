#include "yuca/yuca.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    try {
        if (argc != 3) throw std::runtime_error("expected write/resume/verify and a checkpoint path");
        const std::string mode(argv[1]);
        if (mode == "write") {
            yuca::Indexer index(":title");
            yuca::Document document("session-source");
            document.addKey(yuca::StringKey("hello", ":title"));
            document.stringProperty("title", "hello");
            index.indexDocument(document);
            index.save(argv[2]);
        } else if (mode == "resume") {
            auto index = yuca::Indexer::load(argv[2]);
            if (index.search("hello").size() != 1 || index.getDocument("session-source").stringProperty("title") != "hello")
                throw std::runtime_error("cross-process checkpoint lost string IDs/groups/properties");
            if (!index.removeDocument("session-source")) throw std::runtime_error("cannot remove restored string ID");
            yuca::Document next(7); next.addKey(yuca::StringKey("next", ":title"));
            index.indexDocument(next); index.save(argv[2]);
        } else if (mode == "verify") {
            auto index = yuca::Indexer::load(argv[2]);
            if (!index.search("hello").isEmpty() || index.search("next").size() != 1 || index.getDocument(7).getId() != 7)
                throw std::runtime_error("incremental state did not survive a third process");
        } else throw std::runtime_error("unknown session test mode");
    } catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
