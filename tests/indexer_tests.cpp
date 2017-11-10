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

        fooKey = Key(fooTag);
        fooKey2 = Key(fooTag);
        barKey = Key(barTag);

        documentFoo = Document();
        documentFoo.addKey(fooKey);
        documentFoo.addKey(fooKey2);

        documentBar = Document();
        documentBar.addKey(barKey);
    }

    string fooTag;
    Key fooKey;
    Key fooKey2;

    string barTag;
    Key barKey;

    Document documentFoo;
    Document documentBar;
};


TEST_F(IndexerTests, TestIndexDocument) {
    Indexer indexer;
    indexer.indexDocument(documentFoo);


    ASSERT_TRUE(indexer.findDocuments(fooKey).size() == 1);
    ASSERT_TRUE(indexer.findDocuments(barKey).size() == 0);
    ASSERT_TRUE(indexer.findDocuments(fooKey2).size() == 1);

    indexer.indexDocument(documentBar);

    ASSERT_TRUE(indexer.findDocuments(barKey).size() == 1);

    DocumentSet fooFoundDocs = indexer.findDocuments(fooKey);
    DocumentSet::iterator fooIterator = fooFoundDocs.begin();
    ASSERT_TRUE(fooIterator != fooFoundDocs.end());
    ASSERT_TRUE(*fooIterator == &documentFoo);
    ASSERT_TRUE(*fooIterator != &documentBar);

    DocumentSet barFoundDocs = indexer.findDocuments(barKey);
    DocumentSet::iterator barIterator = barFoundDocs.begin();
    ASSERT_TRUE(barIterator != barFoundDocs.end());
    ASSERT_TRUE(*barIterator == &documentBar);
    ASSERT_TRUE(*barIterator != &documentFoo);

    // search by the other key
    Document *foundDoc = *(indexer.findDocuments(fooKey2).begin());
    ASSERT_TRUE(foundDoc == &documentFoo);

    // add foo and bar key, index should now return 2 results by both keys
    Document fooBarDoc;
    fooBarDoc.addKey(fooKey);
    fooBarDoc.addKey(barKey);
    indexer.indexDocument(fooBarDoc);


    DocumentSet fooDocs = indexer.findDocuments(fooKey);
    ASSERT_TRUE(fooDocs.size() == 2);

    DocumentSet barDocs = indexer.findDocuments(barKey);
    ASSERT_TRUE(barDocs.size() == 2);

    fooIterator = fooDocs.begin();
    ASSERT_TRUE(*fooIterator == &documentFoo);
    fooIterator++;
    ASSERT_TRUE(*fooIterator == &fooBarDoc);
    fooIterator++;
    ASSERT_TRUE(fooIterator == fooDocs.end());

    barIterator = barDocs.begin();
    ASSERT_TRUE(*barIterator == &documentBar);
    barIterator++;
    ASSERT_TRUE(*barIterator == &fooBarDoc);
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
        indexerMultiKey.indexDocument(**fooIterator);
        fooIterator++;
        nDocsIndexed++;
    }
    while (barIterator != barDocs.end()) {
        indexerMultiKey.indexDocument(**barIterator);
        barIterator++;
        nDocsIndexed++;
    }
    Key keys[] = { fooKey, barKey };
    DocumentSet multiIndexDocSet = indexerMultiKey.findDocuments(2, keys);
    std::cout << "Docs found: " << multiIndexDocSet.size();
    ASSERT_TRUE(multiIndexDocSet.size() == nDocsIndexed);
}

#endif