#include "yuca/yuca.hpp"
#include <utility>

int main() {
    yuca::Indexer original(":title");
    yuca::Document document(42);
    document.addKey(yuca::StringKey("hello", ":title"));
    document.stringProperty("title", "original");
    original.indexDocument(document);
    yuca::Indexer copied(original);
    auto source = original.search("hello");
    auto clone = copied.search("hello");
    if (source.size() != 1 || clone.size() != 1 || source.get(0).document_sp == clone.get(0).document_sp) {
        return 1;
    }
    clone.get(0).document_sp->stringProperty("title", "copy");
    if (source.get(0).document_sp->stringProperty("title") != "original") {
        return 1;
    }
    auto source_keys = source.get(0).document_sp->getGroupSPKeys(":title");
    auto copied_keys = clone.get(0).document_sp->getGroupSPKeys(":title");
    if (*source_keys.getStdSet().begin() == *copied_keys.getStdSet().begin()) {
        return 1;
    }
    copied.clear();
    **copied_keys.getStdSet().begin() = yuca::StringKey("changed", ":title");
    if ((**source_keys.getStdSet().begin()).getString() != "hello") {
        return 1;
    }
    if (!copied.search("hello").isEmpty() || original.search("hello").size() != 1) {
        return 1;
    }
    yuca::Indexer another(original);
    yuca::Indexer moved(std::move(another));
    return moved.search("hello").size() == 1 ? 0 : 1;
}
