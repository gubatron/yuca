#include "tests_includes.hpp"

using namespace yuca;

TEST_CASE("Null documents have defined insertion and removal behavior", "[regression]") {
    Indexer indexer;
    CHECK_THROWS_AS(indexer.indexDocument(SPDocument()), std::invalid_argument);
    CHECK_NOTHROW(indexer.removeDocument(SPDocument()));
}

TEST_CASE("Replacing a document removes its previous postings", "[regression]") {
    Indexer indexer;
    Document old_doc(42);
    old_doc.addKey(StringKey("old", ":keyword"));
    old_doc.addKey(StringKey("txt", ":extension"));
    indexer.indexDocument(old_doc);
    Document replacement(42);
    replacement.addKey(StringKey("new", ":keyword"));
    indexer.indexDocument(replacement);
    indexer.indexDocument(replacement);
    CHECK(indexer.search("old").isEmpty());
    CHECK(indexer.search(":extension txt").isEmpty());
    REQUIRE(indexer.search("new").size() == 1);
    CHECK(indexer.removeDocument(42));
    CHECK(indexer.search("new").isEmpty());
    CHECK_FALSE(indexer.removeDocument(42));
}

TEST_CASE("Pointer removal uses the cached document's keys", "[regression]") {
    Indexer indexer;
    Document doc(42);
    doc.addKey(StringKey("hello", ":keyword"));
    indexer.indexDocument(doc);
    indexer.removeDocument(std::make_shared<Document>(42));
    CHECK(indexer.search("hello").isEmpty());
    CHECK(indexer.getDocument(42) == Document::NULL_DOCUMENT);
}

TEST_CASE("The sentinel document id can still be removed", "[regression]") {
    Indexer indexer;
    Document doc(-1);
    doc.addKey(StringKey("hello", ":keyword"));
    indexer.indexDocument(doc);
    CHECK(indexer.removeDocument(-1));
    CHECK(indexer.search("hello").isEmpty());
}
