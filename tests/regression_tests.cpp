#include "tests_includes.hpp"
#include <limits>

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

TEST_CASE("Collection bounds do not wrap", "[regression]") {
    utils::List<int> list;
    list.add(7);
    CHECK(list.subList(std::numeric_limits<unsigned long>::max(), 2).isEmpty());
    CHECK(list.subList(0, std::numeric_limits<unsigned long>::max()).size() == 1);
    CHECK_THROWS_AS(list.add(-1, 9), std::out_of_range);
    CHECK_THROWS_AS(list.add(2, 9), std::out_of_range);
    CHECK_THROWS_AS(list.set(-1, 9), std::out_of_range);
}

TEST_CASE("Temporary collection access owns its storage", "[regression]") {
    Document doc(1);
    doc.stringProperty("title", "hello");
    unsigned count = 0;
    for (const auto &key : doc.propertyKeys(STRING).getStdVector()) {
        CHECK(key == "title");
        ++count;
    }
    CHECK(count == 1);
    for (const auto &key : doc.getGroupKeys(":missing").getStdSet()) {
        (void)key;
        FAIL("missing group must be empty");
    }
    utils::Set<int> set;
    set.add(9);
    for (int value : utils::Set<int>(set).getStdSet()) {
        CHECK(value == 9);
    }
    utils::Map<int, int> map(0);
    map.put(1, 2);
    for (const auto &entry : utils::Map<int, int>(map).getStdMap()) {
        CHECK(entry.first == 1);
        CHECK(entry.second == 2);
    }
}

TEST_CASE("Collection self operations preserve valid iterators", "[regression]") {
    utils::List<int> list;
    list.add(1);
    list.add(2);
    list.addAll(list);
    REQUIRE(list.size() == 4);
    CHECK(list.get(2) == 1);
    CHECK(list.get(3) == 2);
    CHECK(list.removeAll(list));
    CHECK(list.isEmpty());
    utils::Set<int> set;
    set.add(1);
    set.add(2);
    set.removeAll(set);
    CHECK(set.isEmpty());
}

namespace {
    struct ThrowingCopy {
        bool *fail;
        explicit ThrowingCopy(bool *flag) : fail(flag) {}
        ThrowingCopy(const ThrowingCopy &other) : fail(other.fail) {
            if (*fail) {
                throw std::runtime_error("test copy failure");
            }
        }
    };
}

TEST_CASE("Collection copy failures propagate instead of terminating", "[regression]") {
    bool fail = false;
    ThrowingCopy value(&fail);
    utils::List<ThrowingCopy> list;
    list.add(value);
    utils::Map<int, ThrowingCopy> map(value);
    fail = true;
    CHECK_THROWS_AS(list.add(value), std::runtime_error);
    CHECK_THROWS_AS(list.getStdVectorCopy(), std::runtime_error);
    CHECK_THROWS_AS(map.get(0), std::runtime_error);
}

TEST_CASE("Whitespace is not a search keyword", "[regression]") {
    SearchRequest request(" \t hello  \n :extension\t txt \r\n", ":keyword");
    CHECK(request.total_keywords == 2);
    CHECK(request.getGroups().size() == 2);
    Indexer indexer;
    Document doc(1);
    doc.addKey(StringKey("hello", ":keyword"));
    doc.addKey(StringKey("txt", ":extension"));
    indexer.indexDocument(doc);
    CHECK(indexer.search(" \t hello  \n :extension\t txt \r\n").size() == 1);
    CHECK(indexer.search(" \t\n ").isEmpty());
    CHECK(indexer.search("hello :missing nope").isEmpty());
}

TEST_CASE("Levenshtein ranking preserves fractional scores", "[regression]") {
    Indexer indexer;
    Document near_doc(1);
    near_doc.addKey(StringKey("cat", ":keyword"));
    near_doc.stringProperty("title", "coatx");
    Document far_doc(2);
    far_doc.addKey(StringKey("cat", ":keyword"));
    far_doc.stringProperty("title", "zzzzz");
    indexer.indexDocument(far_doc);
    indexer.indexDocument(near_doc);
    auto results = indexer.search("cat", "title");
    REQUIRE(results.size() == 2);
    CHECK(results.get(0).document_sp->getId() == 1);
    CHECK(results.get(0).score == Approx(1.5));
    CHECK(results.get(1).score == Approx(1.2));
}

TEST_CASE("Search result identifiers do not overflow for large document ids", "[regression]") {
    auto request = std::make_shared<SearchRequest>("hello", ":keyword");
    auto doc = std::make_shared<Document>(std::numeric_limits<long>::max());
    SearchResult result(request, doc);
    CHECK(result.id >= 0);
    CHECK(result.document_sp->getId() == std::numeric_limits<long>::max());
}

TEST_CASE("Posting updates retain canonical keys and independent result snapshots", "[regression]") {
    ReverseIndex reverse;
    auto key = std::make_shared<StringKey>("common", ":keyword");
    auto equivalent = std::make_shared<StringKey>("common", ":keyword");
    auto first = std::make_shared<Document>(1);
    auto second = std::make_shared<Document>(2);
    reverse.putDocument(key, first);
    auto snapshot = reverse.getDocuments(equivalent);
    reverse.putDocument(equivalent, second);
    reverse.putDocument(equivalent, second);
    CHECK(reverse.getKeyCount() == 1);
    CHECK(reverse.keyCacheGet(equivalent) == key);
    CHECK(reverse.getDocuments(key).size() == 2);
    CHECK(snapshot.size() == 1);
    reverse.removeDocument(equivalent, first);
    CHECK(reverse.hasDocuments(key));
    CHECK(reverse.getDocuments(key).size() == 1);
    reverse.removeDocument(key, second);
    CHECK_FALSE(reverse.hasDocuments(equivalent));
    CHECK(reverse.getKeyCount() == 0);
    reverse.removeDocument(key, first);
    CHECK(reverse.getKeyCount() == 0);
}
