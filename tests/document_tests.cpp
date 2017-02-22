//
// Created by gubatron on 11/19/2016.
//
#ifndef YUCA_DOCUMENT_TESTS_H
#define YUCA_DOCUMENT_TESTS_H

#include <gtest/gtest.h>
#include <yuca/key.hpp>
#include <yuca/document.hpp>

using namespace yuca;

class DocumentTests : public ::testing::Test {
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


TEST_F(DocumentTests, TestIfKeyCanBeAdded) {

    KeySet isEmpty = document.getTagKeys(fooTag);
    ASSERT_TRUE(isEmpty.empty());

    document.addKey(fooKey);
    document.addKey(barKey);
    document.addKey(barKey2);


    KeySet fooKeys = document.getTagKeys(fooTag);
    ASSERT_TRUE(fooKeys.size() == 1);

    KeySet barKeys = document.getTagKeys(barTag);
    ASSERT_TRUE(barKeys.size() == 2);

    // make sure it's the same one we've added
    KeySet::iterator it = fooKeys.find(&fooKey);

    ASSERT_FALSE(it == fooKeys.end());

    Key *isThisFooKeyPtr = *it;
    std::cout << "tag found? " << isThisFooKeyPtr->getTag() << std::endl;

    ASSERT_TRUE(isThisFooKeyPtr->getTag() == fooTag);
    ASSERT_TRUE(isThisFooKeyPtr == (Key*) &fooKey);

    it = barKeys.find(&barKey);
    Key *isThisBarKeyPtr = *it;
    std::cout << "tag found? " << isThisBarKeyPtr->getTag() << std::endl;
    ASSERT_TRUE(isThisBarKeyPtr->getTag() == barTag);
    ASSERT_TRUE(isThisBarKeyPtr == (Key*) &barKey);

    it++;
    Key *isThisBarkey2Ptr = *it;
    ASSERT_TRUE(isThisBarkey2Ptr->getTag() == barTag);
    ASSERT_TRUE(isThisBarkey2Ptr == (Key*) &barKey2);


    // now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
    document.addKey(barKey);
    barKeys = document.getTagKeys(barTag);
    ASSERT_TRUE(barKeys.size() == 2);
}

TEST_F(DocumentTests, TestsIfSingleKeyCanBeRemoved) {
    document.addKey(fooKey);
    document.addKey(barKey);
    KeySet fooKeySet = document.getTagKeys(fooTag);
    ASSERT_TRUE(fooKeySet.size() == 1);

    KeySet barKeySet = document.getTagKeys(barTag);
    ASSERT_TRUE(barKeySet.size() == 1);
    document.addKey(barKey2);
    ASSERT_TRUE(document.getTagKeys(barTag).size() == 2);

    document.removeKey(fooTag, fooKey);
    fooKeySet = document.getTagKeys(fooTag);
    ASSERT_TRUE(fooKeySet.size() == 0);

    KeySet::iterator it_not_there = fooKeySet.find(&fooKey);
    ASSERT_TRUE(it_not_there == fooKeySet.end());

    barKeySet = document.getTagKeys(barTag);
    KeySet::iterator bar_there = barKeySet.find(&barKey);
    ASSERT_TRUE(*bar_there == (Key*) &barKey);

    KeySet::iterator bar2_there = barKeySet.find(&barKey2);
    ASSERT_TRUE(bar2_there != barKeySet.end());
    ASSERT_TRUE(*bar2_there == (Key*) &barKey2);
}

TEST_F(DocumentTests, TestGetTags) {
    std::string zeeTag = "zeeTag:";
    Key zeeKey(zeeTag);
    document.addKey(zeeKey);
    document.addKey(fooKey);
    document.addKey(barKey);

    std::cout << "Getting all the tags...";

    std::vector<string> tags = document.getTags();
    std::cout << "Got " << tags.size() << " tags" << std::endl;
    std::cout << "tags[0] => " << tags.at(0) << std::endl;
    std::cout << "tags[1] => " << tags.at(1) << std::endl;
    std::cout << "tags[2] => " << tags.at(2) << std::endl;

    // elements will be sorted as they're inserted in the set
    // they are not in the set in the order they were inserted.
    ASSERT_TRUE(tags[0] == barTag);
    ASSERT_TRUE(tags[1] == fooTag);
    ASSERT_TRUE(tags[2] == zeeTag);
}

TEST_F(DocumentTests, TestRemoveTag) {
    std::string nameTag = "name:";
    Key nameKey = Key(nameTag);
    document.addKey(nameKey);
    document.addKey(barKey);
    document.addKey(barKey2);
    KeySet nameTagKeys = document.getTagKeys("name:");
    KeySet::iterator foundKeys = nameTagKeys.find(&barKey);
    ASSERT_TRUE(foundKeys == nameTagKeys.end()); //assert not found
    foundKeys = nameTagKeys.find(&nameKey);
    ASSERT_FALSE(foundKeys == nameTagKeys.end());
    ASSERT_TRUE(*foundKeys == &nameKey);
    document.removeKey(nameTag, nameKey);
    nameTagKeys = document.getTagKeys("name:");
    ASSERT_TRUE(nameTagKeys.empty());
    KeySet barKeys = document.getTagKeys(barKey.getTag());
    ASSERT_TRUE(barKeys.size() == 2);
    document.removeTag(barKey.getTag());
    barKeys = document.getTagKeys(barKey.getTag());
    ASSERT_TRUE(barKeys.empty());
}

#endif
