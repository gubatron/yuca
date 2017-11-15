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
    void SetUp() {
        fooTag = ":foo";
        barTag = ":bar";

        fooKey = Key(1,fooTag);
        barKey = Key(1, barTag);
        barKey2 = Key(2, barTag);
        document = Document();
    }

    std::string fooTag;
    Key fooKey;
    std::string barTag;

    Key barKey;
    Key barKey2;
    Document document;
};


TEST_F(DocumentTests, TestIfKeyCanBeAdded) {

    KeySet isEmpty;
    document.getTagKeys(fooTag, isEmpty);
    ASSERT_TRUE(isEmpty.empty());

    auto fooKey_sp = std::make_shared<Key>(fooKey);
    document.addKey(fooKey_sp);
    KeySet fooKeys;
    document.getTagKeys(fooTag, fooKeys);
    ASSERT_TRUE(fooKeys.size() == 1);
    if (barKey < barKey2) {
        std::cout << "barKey < barKey2 indeed" << std::endl;
    } else {
        FAIL();
    }

    auto barKey_sp = std::make_shared<Key>(barKey);
    auto barKey2_sp = std::make_shared<Key>(barKey2);
    document.addKey(barKey_sp);
    document.addKey(barKey2_sp);

    KeySet barKeys;
    document.getTagKeys(barTag, barKeys);
    ASSERT_TRUE(barKeys.size() == 2);

    // make sure it's the same one we've added

    auto it = fooKeys.find(fooKey_sp);
    if (it == fooKeys.end()) {
        std::cout << "Couldn't find fooKey even though we just added it. What the hell do we have here then?" << std::endl;
        it = fooKeys.begin();
        (*it)->dumpToStream(std::cout);
        std::cout << std::endl << "vs" << std::endl;
        fooKey.dumpToStream(std::cout);
        std::cout << std::endl << "------------------------" << std::endl;
        FAIL();
    }
    ASSERT_TRUE(fooKeys.size() == 1);
    ASSERT_FALSE(it == fooKeys.end());
    ASSERT_TRUE(**it == fooKey);
    ASSERT_TRUE((*it)->getTag() == fooTag);

    it = barKeys.find(barKey_sp);
    Key isThisBarKey = **it;
    std::cout << "tag found? " << isThisBarKey.getTag() << std::endl;
    ASSERT_TRUE(isThisBarKey.getTag() == barTag);
    ASSERT_TRUE(isThisBarKey == barKey);

    it++;
    Key isThisBarkey2 = **it;
    ASSERT_TRUE(isThisBarkey2.getTag() == barTag);
    ASSERT_TRUE(isThisBarkey2 == barKey2);

    // now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
    document.addKey(barKey_sp);
    document.getTagKeys(barTag, barKeys);
    ASSERT_TRUE(barKeys.size() == 2);
}

TEST_F(DocumentTests, TestsIfSingleKeyCanBeRemoved) {
    auto fooKey_sp = std::make_shared<Key>(fooKey);
    auto barKey_sp = std::make_shared<Key>(barKey);
    document.addKey(fooKey_sp);
    document.addKey(barKey_sp);
    KeySet fooKeySet;
    document.getTagKeys(fooTag, fooKeySet);
    ASSERT_TRUE(fooKeySet.size() == 1);

    KeySet barKeySet;
    document.getTagKeys(barTag, barKeySet);
    ASSERT_TRUE(barKeySet.size() == 1);

    auto barKey2_sp = std::make_shared<Key>(barKey2);
    document.addKey(barKey2_sp);
    document.getTagKeys(barTag, barKeySet);
    ASSERT_TRUE(barKeySet.size() == 2);

    document.removeKey(fooTag, fooKey_sp);
    document.getTagKeys(fooTag, fooKeySet);
    ASSERT_TRUE(fooKeySet.size() == 0);

    KeySet::iterator it_not_there = fooKeySet.find(fooKey_sp);
    ASSERT_TRUE(it_not_there == fooKeySet.end());

    document.getTagKeys(barTag, barKeySet);
    KeySet::iterator bar_there = barKeySet.find(barKey_sp);
    ASSERT_TRUE(**bar_there == barKey);

    KeySet::iterator bar2_there = barKeySet.find(barKey2_sp);
    ASSERT_TRUE(bar2_there != barKeySet.end());
    ASSERT_TRUE(**bar2_there == barKey2);
}

TEST_F(DocumentTests, TestGetTags) {
    std::string zeeTag = ":zeeTag";
    Key zeeKey(7, zeeTag);
    auto zeeKey_sp = std::make_shared<Key>(zeeKey);
    auto fooKey_sp = std::make_shared<Key>(fooKey);
    auto barKey_sp = std::make_shared<Key>(barKey);

    document.addKey(zeeKey_sp);
    document.addKey(fooKey_sp);
    document.addKey(barKey_sp);

    std::cout << "Getting all the tags...";

    std::set<std::string> tags;
    document.getTags(tags);
    std::cout << "Got " << tags.size() << " tags" << std::endl;
    auto it = tags.begin();
    std::cout << "tags[0] => " << *it++ << std::endl;
    std::cout << "tags[1] => " << *it++ << std::endl;
    std::cout << "tags[2] => " << *it << std::endl;

    // elements will be sorted as they're inserted in the set
    // they are not in the set in the order they were inserted.
    it = tags.begin();
    ASSERT_TRUE(*it++ == barTag);
    ASSERT_TRUE(*it++ == fooTag);
    ASSERT_TRUE(*it == zeeTag);
}

TEST_F(DocumentTests, TestRemoveTag) {
    std::string nameTag = ":name";
    Key nameKey = Key(4, nameTag);
    auto nameKey_sp = std::make_shared<Key>(nameKey);
    auto barKey_sp = std::make_shared<Key>(barKey);
    auto barKey2_sp = std::make_shared<Key>(barKey2);
    document.addKey(nameKey_sp);
    document.addKey(barKey_sp);
    document.addKey(barKey2_sp);

    KeySet nameTagKeys;
    document.getTagKeys(":name", nameTagKeys);
    KeySet::iterator foundKeys = nameTagKeys.find(barKey_sp);
    ASSERT_TRUE(foundKeys == nameTagKeys.end()); //assert not found
    foundKeys = nameTagKeys.find(nameKey_sp);
    ASSERT_FALSE(foundKeys == nameTagKeys.end());
    ASSERT_TRUE(**foundKeys == nameKey);
    ASSERT_TRUE(foundKeys == nameTagKeys.begin());
    document.dumpToStream(std::cout);
    document.removeKey(nameTag, nameKey_sp);
    std::cout << std::endl << "after :name key removed" << std::endl;
    document.dumpToStream(std::cout);
    document.getTagKeys(":name", nameTagKeys);
    ASSERT_TRUE(nameTagKeys.empty());
    KeySet barKeys;
    document.getTagKeys(barKey.getTag(), barKeys);
    ASSERT_TRUE(barKeys.size() == 2);
    document.removeTag(barKey.getTag());
    document.getTagKeys(barKey.getTag(), barKeys);
    ASSERT_TRUE(barKeys.empty());
}

#endif
