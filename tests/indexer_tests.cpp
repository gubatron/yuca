//
// Created by gubatron on 02/21/2017.
//

#ifndef YUCA_INDEXER_TESTS_H
#define YUCA_INDEXER_TESTS_H

#include <gtest/gtest.h>
#include <yuca/key.hpp>
#include <yuca/document.hpp>

using namespace yuca;

class IndexerTests : public ::testing::Test {
public:
    virtual void SetUp() {
        fooTag = ":foo";
        barTag = ":bar";

        fooKey = Key(fooTag);
        barKey = Key(barTag);
        barKey2 = Key(barTag);
        document = Document();
    }

    string fooTag;
    Key fooKey;
    string barTag;

    Key barKey;
    Key barKey2;
    Document document;
};


TEST_F(IndexerTests, TestA) {
}

#endif