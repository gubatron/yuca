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
        fooTag = ":foo";
        barTag = ":bar";

        fooKey = Key(1, fooTag);
        fooKey2 = Key(2, fooTag);
        barKey = Key(1, barTag);

        fooKey_sp = std::make_shared<Key>(fooKey);
        fooKey2_sp = std::make_shared<Key>(fooKey2);
        barKey_sp = std::make_shared<Key>(barKey);

        documentFoo = Document();
        documentFoo.addKey(fooKey_sp);
        documentFoo.addKey(fooKey2_sp);
        documentFoo_sp = std::make_shared<Document>(documentFoo);

        documentBar = Document();
        documentBar.addKey(barKey_sp);
        documentBar_sp = std::make_shared<Document>(documentBar);

        // A Document can have many keys.
        // Each key has to have a tag, which serves as a search dimension parameter
        // The indexer indexes documents with its keys.
        // Documents store their own keys
        // And the indexer keeps a reverse index which maps keys to sets of documents.
    }

    std::string fooTag;
    Key fooKey;
    Key fooKey2;
    std::shared_ptr<Key> fooKey_sp;
    std::shared_ptr<Key> fooKey2_sp;

    std::string barTag;
    Key barKey;
    std::shared_ptr<Key> barKey_sp;

    Document documentFoo;
    Document documentBar;
    std::shared_ptr<Document> documentFoo_sp;
    std::shared_ptr<Document> documentBar_sp;
};


TEST_F(IndexerTests, TestIndexDocument) {
    Indexer indexer;
    documentFoo.dumpToStream(std::cout);
    std::set<std::string> someTags;
    documentFoo.getTags(someTags);
    std::set<std::string> someTags_from_sp;
    documentFoo_sp.get()->getTags(someTags_from_sp);
    indexer.indexDocument(documentFoo_sp);
    indexer.dumpToStream(std::cout);

    DocumentSet fooKeyDocs;
    indexer.findDocuments(fooKey_sp, fooKeyDocs);

    ASSERT_TRUE(fooKeyDocs.size() == 1);

    DocumentSet barKeyDocs;
    indexer.findDocuments(barKey_sp, barKeyDocs);
    ASSERT_TRUE(barKeyDocs.size() == 0);

    DocumentSet fooKey2Docs;
    indexer.findDocuments(fooKey2_sp, fooKey2Docs);
    ASSERT_TRUE(fooKey2Docs.size() == 1);

    indexer.indexDocument(documentBar_sp);
    indexer.findDocuments(barKey_sp, barKeyDocs);
    ASSERT_TRUE(barKeyDocs.size() == 1);

    // search by the first key (fooKey)
    DocumentSet fooFoundDocs;
    indexer.findDocuments(fooKey_sp, fooFoundDocs);
    auto fooIterator = fooFoundDocs.begin();
    ASSERT_TRUE(fooIterator != fooFoundDocs.end());
    ASSERT_TRUE(**fooIterator == documentFoo);
    ASSERT_FALSE(**fooIterator == documentBar);

    DocumentSet barFoundDocs;
    indexer.findDocuments(barKey_sp, barFoundDocs);
    DocumentSet::iterator barIterator = barFoundDocs.begin();
    ASSERT_TRUE(barIterator != barFoundDocs.end());
    Document firstBarFoundDoc = **barIterator;
    ASSERT_TRUE(firstBarFoundDoc == documentBar);
    ASSERT_FALSE(firstBarFoundDoc == documentFoo);

    // search by the 2nd key (fooKey2)
    indexer.findDocuments(fooKey2_sp, fooKey2Docs);
    Document foundDoc = **fooKey2Docs.begin();
    ASSERT_TRUE(foundDoc == documentFoo);

    // add foo and bar key, index should now return 2 results by both keys
    Document fooBarDoc;
    fooBarDoc.addKey(fooKey_sp);
    fooBarDoc.addKey(barKey_sp);
    std::shared_ptr<Document> fooBarDoc_sp = std::make_shared<Document>(fooBarDoc);
    indexer.indexDocument(fooBarDoc_sp);


    DocumentSet fooDocs;
    indexer.findDocuments(fooKey_sp, fooDocs);
    ASSERT_TRUE(fooDocs.size() == 2);

    DocumentSet barDocs;
    indexer.findDocuments(barKey_sp, barDocs);
    ASSERT_TRUE(barDocs.size() == 2);

    fooIterator = fooDocs.begin();
    ASSERT_TRUE(**fooIterator == documentFoo);
    fooIterator++;
    ASSERT_TRUE(**fooIterator == fooBarDoc);
    fooIterator++;
    ASSERT_TRUE(fooIterator == fooDocs.end());

    barIterator = barDocs.begin();
    ASSERT_TRUE(**barIterator == documentBar);
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
    std::shared_ptr<Key> keys[] = { fooKey_sp, barKey_sp };
    DocumentSet multiIndexDocSet;
    indexerMultiKey.findDocuments(2, keys, multiIndexDocSet);
    std::cout << "Docs found: " << multiIndexDocSet.size();
    ASSERT_TRUE(multiIndexDocSet.size() == nDocsIndexed);
}

#endif