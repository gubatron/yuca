#include "yuca/yuca.hpp"
#include <iostream>

// Complete C++11 example, compiled and executed by CTest.
int main() {
    yuca::Indexer index;
    yuca::Document document(42);
    document.addKey(yuca::StringKey("hello", ":keyword"));
    document.addKey(yuca::StringKey("txt", ":extension"));
    document.stringProperty("title", "hello yuca");
    index.indexDocument(document);

    const auto results = index.search("hello :extension txt");
    if (results.size() != 1 || results.get(0).document_sp->getId() != 42 || results.get(0).score != 2.0) {
        return 1;
    }
    std::cout << results.get(0).document_sp->getId() << ' '
              << results.get(0).document_sp->stringProperty("title") << '\n';

    yuca::Document replacement(42);
    replacement.addKey(yuca::StringKey("goodbye", ":keyword"));
    index.indexDocument(replacement);
    if (!index.search("hello").isEmpty() || index.search("goodbye").size() != 1) {
        return 1;
    }
    if (!index.removeDocument(42) || !index.search("goodbye").isEmpty()) {
        return 1;
    }
    return 0;
}
