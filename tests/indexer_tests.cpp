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
        barKey_sp = std::make_shared<Key>(1, bar_tag);

        document_foo_sp = std::make_shared<Document>();
        document_foo_sp->addKey(foo_key_sp);
        document_foo_sp->addKey(foo_key2_sp);


        document_bar_sp = std::make_shared<Document>();
        document_bar_sp->addKey(barKey_sp);
        

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
    std::shared_ptr<Key> barKey_sp;

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

    DocumentSet fooKeyDocs;
    indexer.findDocuments(foo_key_sp, fooKeyDocs);

    ASSERT_TRUE(fooKeyDocs.size() == 1);

    DocumentSet barKeyDocs;
    indexer.findDocuments(barKey_sp, barKeyDocs);
    ASSERT_TRUE(barKeyDocs.size() == 0);

    DocumentSet fooKey2Docs;
    indexer.findDocuments(foo_key2_sp, fooKey2Docs);
    ASSERT_TRUE(fooKey2Docs.size() == 1);

    indexer.indexDocument(document_bar_sp);
    indexer.findDocuments(barKey_sp, barKeyDocs);
    ASSERT_TRUE(barKeyDocs.size() == 1);

    // search by the first key (fooKey)
    DocumentSet fooFoundDocs;
    indexer.findDocuments(foo_key_sp, fooFoundDocs);
    auto fooIterator = fooFoundDocs.begin();
    ASSERT_TRUE(fooIterator != fooFoundDocs.end());
    ASSERT_TRUE(**fooIterator == *document_foo_sp.get());
    ASSERT_FALSE(**fooIterator == *document_bar_sp.get());

    DocumentSet barFoundDocs;
    indexer.findDocuments(barKey_sp, barFoundDocs);
    DocumentSet::iterator barIterator = barFoundDocs.begin();
    ASSERT_TRUE(barIterator != barFoundDocs.end());
    Document firstBarFoundDoc = **barIterator;
    ASSERT_TRUE(**firstBarFoundDoc == document_bar_sp.get());
    ASSERT_FALSE(**firstBarFoundDoc == document_foo_sp);

    // search by the 2nd key (fooKey2)
    indexer.findDocuments(foo_key2_sp, fooKey2Docs);
    Document foundDoc = **fooKey2Docs.begin();
    ASSERT_TRUE(foundDoc == document_foo);

    // add foo and bar key, index should now return 2 results by both keys
    Document fooBarDoc;
    fooBarDoc.addKey(foo_key_sp);
    fooBarDoc.addKey(barKey_sp);
    std::shared_ptr<Document> fooBarDoc_sp = std::make_shared<Document>(fooBarDoc);
    indexer.indexDocument(fooBarDoc_sp);


    DocumentSet fooDocs;
    indexer.findDocuments(foo_key_sp, fooDocs);
    ASSERT_TRUE(fooDocs.size() == 2);

    DocumentSet barDocs;
    indexer.findDocuments(barKey_sp, barDocs);
    ASSERT_TRUE(barDocs.size() == 2);

    fooIterator = fooDocs.begin();
    ASSERT_TRUE(**fooIterator == document_foo);
    fooIterator++;
    ASSERT_TRUE(**fooIterator == fooBarDoc);
    fooIterator++;
    ASSERT_TRUE(fooIterator == fooDocs.end());

    barIterator = barDocs.begin();
    ASSERT_TRUE(**barIterator == document_bar);
    barIterator++;
    ASSERT_TRUE(**barIterator == fooBarDoc);
    barIterator++;
    ASSERT_TRUE(barIterator == barDocs.end());

    //////////////////////////////
    // multiple key search tests
    //////////////////////////////

    Indexer indexerMultiKey;
    fooIterator = fooDocs.begin();
    barIterator = barDocs.begin();
    int nDocsIndexed = 0;

    // add foo documents
    while (fooIterator != fooDocs.end()) {
        indexerMultiKey.indexDocument(*fooIterator);
        fooIterator++;
        nDocsIndexed++;
    }
    while (barIterator != barDocs.end()) {
        indexerMultiKey.indexDocument(*barIterator);
        barIterator++;
        nDocsIndexed++;
    }
    std::shared_ptr<Key> keys[] = { foo_key_sp, barKey_sp };
    DocumentSet multiIndexDocSet;
    indexerMultiKey.findDocuments(2, keys, multiIndexDocSet);
    std::cout << "Docs found: " << multiIndexDocSet.size();
    ASSERT_TRUE(multiIndexDocSet.size() == nDocsIndexed);
}

#endif