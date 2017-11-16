//
// Created by gubatron on 02/21/2017.
//

#ifndef YUCA_INDEXER_TESTS_H
#define YUCA_INDEXER_TESTS_H

#include <gtest/gtest.h>
#include <yuca/key.hpp>
#include <yuca/document.hpp>
#include <yuca/indexer.hpp>

using namespace yuca;

class IndexerTests : public ::testing::Test {
public:

    void SetUp() {
        foo_tag = ":foo";
        bar_tag = ":bar";

        foo_key_sp = std::make_shared<Key>(1, foo_tag);
        foo_key2_sp = std::make_shared<Key>(2, foo_tag);
        bar_key_sp = std::make_shared<Key>(1, bar_tag);

        document_foo_sp = std::make_shared<Document>();
        document_foo_sp->addKey(foo_key_sp);
        document_foo_sp->addKey(foo_key2_sp);


        document_bar_sp = std::make_shared<Document>();
        document_bar_sp->addKey(bar_key_sp);
        

        // A Document can have many keys.
        // Each key has to have a tag, which serves as a search dimension parameter
        // The indexer indexes documents with its keys.
        // Documents store their own keys
        // And the indexer keeps a reverse index which maps keys to sets of documents.
    }

    std::string foo_tag;
    std::shared_ptr<Key> foo_key_sp;
    std::shared_ptr<Key> foo_key2_sp;

    std::string bar_tag;
    std::shared_ptr<Key> bar_key_sp;

    std::shared_ptr<Document> document_foo_sp;
    std::shared_ptr<Document> document_bar_sp;
};


TEST_F(IndexerTests, TestIndexDocument) {
    Indexer indexer;
    document_foo_sp->dumpToStream(std::cout);
    std::set<std::string> someTags;
    document_foo_sp->getTags(someTags);
    std::set<std::string> someTags_from_sp;
    document_foo_sp.get()->getTags(someTags_from_sp);
    indexer.indexDocument(document_foo_sp);
    indexer.dumpToStream(std::cout);

    DocumentSet foo_key_docs;
    indexer.findDocuments(foo_key_sp, foo_key_docs);

    ASSERT_TRUE(foo_key_docs.size() == 1);

    DocumentSet bar_key_docs;
    indexer.findDocuments(bar_key_sp, bar_key_docs);
    ASSERT_TRUE(bar_key_docs.size() == 0);

    DocumentSet foo_key2_docs;
    indexer.findDocuments(foo_key2_sp, foo_key2_docs);
    ASSERT_TRUE(foo_key2_docs.size() == 1);

    indexer.indexDocument(document_bar_sp);
    indexer.findDocuments(bar_key_sp, bar_key_docs);
    ASSERT_TRUE(bar_key_docs.size() == 1);

    // search by the first key (fooKey)
    DocumentSet foo_found_docs;
    indexer.findDocuments(foo_key_sp, foo_found_docs);
    auto foo_it = foo_found_docs.begin();
    ASSERT_TRUE(foo_it != foo_found_docs.end());
    ASSERT_TRUE(**foo_it == *document_foo_sp.get());
    ASSERT_FALSE(**foo_it == *document_bar_sp.get());

    DocumentSet bar_found_docs;
    indexer.findDocuments(bar_key_sp, bar_found_docs);
    DocumentSet::iterator bar_it = bar_found_docs.begin();
    ASSERT_TRUE(bar_it != bar_found_docs.end());

    Document &first_bar_found_doc = **bar_it;
    ASSERT_TRUE(first_bar_found_doc == *document_bar_sp);
    ASSERT_FALSE(first_bar_found_doc == *document_foo_sp);

    // search by the 2nd key (fooKey2)
    indexer.findDocuments(foo_key2_sp, foo_key2_docs);
    Document &found_doc = **foo_key2_docs.begin();
    ASSERT_TRUE(found_doc == *document_foo_sp);

    // add foo and bar key, index should now return 2 results by both keys
    std::shared_ptr<Document> foo_bar_doc_sp = std::make_shared<Document>();
    foo_bar_doc_sp->addKey(foo_key_sp);
    foo_bar_doc_sp->addKey(bar_key_sp);

    indexer.indexDocument(foo_bar_doc_sp);

    DocumentSet foo_docs;
    indexer.findDocuments(foo_key_sp, foo_docs);
    ASSERT_TRUE(foo_docs.size() == 2);

    DocumentSet bar_docs;
    indexer.findDocuments(bar_key_sp, bar_docs);
    ASSERT_TRUE(bar_docs.size() == 2);

    foo_it = foo_docs.begin();
    ASSERT_TRUE(**foo_it == *document_foo_sp);
    foo_it++;
    ASSERT_TRUE(**foo_it == *foo_bar_doc_sp);
    foo_it++;
    ASSERT_TRUE(foo_it == foo_docs.end());

    bar_it = bar_docs.begin();
    ASSERT_TRUE(**bar_it == *document_bar_sp);
    bar_it++;
    ASSERT_TRUE(**bar_it == *foo_bar_doc_sp);
    bar_it++;
    ASSERT_TRUE(bar_it == bar_docs.end());

    //////////////////////////////
    // multiple key search tests
    //////////////////////////////

    Indexer indexer_multi_key;
    foo_it = foo_docs.begin();
    bar_it = bar_docs.begin();
    DocumentSet indexed_docs_checker;
    int n_docs_indexed = 0;

    // add foo documents
    while (foo_it != foo_docs.end()) {
        indexer_multi_key.indexDocument(*foo_it);
        indexed_docs_checker.insert(*foo_it);
        (**foo_it).dumpToStream(std::cout);
        foo_it++;
        n_docs_indexed = indexed_docs_checker.size();
    }
    while (bar_it != bar_docs.end()) {
        indexer_multi_key.indexDocument(*bar_it);
        indexed_docs_checker.insert(*bar_it);
        (**bar_it).dumpToStream(std::cout);
        bar_it++;
        n_docs_indexed = indexed_docs_checker.size();
    }
    std::shared_ptr<Key> keys[] = { foo_key_sp, bar_key_sp };
    DocumentSet multi_index_doc_set;
    indexer_multi_key.findDocuments(2, keys, multi_index_doc_set);
    std::cout << "Docs found: " << multi_index_doc_set.size();

    ASSERT_TRUE(multi_index_doc_set.size() == n_docs_indexed);
}

#endif