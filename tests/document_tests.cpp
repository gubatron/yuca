//
// Created by gubatron on 11/19/16.
//

#include <gtest/gtest.h>
#include <yuca/document.hpp>

using namespace yuca;

class DocumentTests : public ::testing::Test {
public:
    virtual void SetUp() {
        fooTag = "foo:";
        fooKey = Key(fooTag);
        barTag = "bar:";
        barKey = Key(barTag);
        document = Document();
    }

    string fooTag;
    Key fooKey;
    string barTag;
    Key barKey;
    Document document;
};


TEST_F(DocumentTests, TestIfKeyCanBeAdded) {

    KeySet isEmpty = document.getTagKeys(fooTag);
    ASSERT_TRUE(isEmpty.empty());

    document.addKey(fooTag, fooKey);
    document.addKey(fooTag, barKey);

    KeySet fooKeys = document.getTagKeys(fooTag);

    // make sure we have two
    ASSERT_TRUE(fooKeys.size() == 2);

    // make sure it's the same one we've added
    KeySet::iterator it = fooKeys.find(&fooKey);

    ASSERT_FALSE(it == fooKeys.end());

    Key *isThisFooKeyPtr = *it;
    std::cout << "tag found? " << isThisFooKeyPtr->getTag() << std::endl;

    ASSERT_TRUE(isThisFooKeyPtr->getTag() == fooTag);
    ASSERT_TRUE(isThisFooKeyPtr == (Key*) &fooKey);

    it++;
    Key *isThisBarKeyPtr = *it;
    std::cout << "tag found? " << isThisBarKeyPtr->getTag() << std::endl;

    ASSERT_TRUE(isThisBarKeyPtr->getTag() == barTag);
    ASSERT_TRUE(isThisBarKeyPtr == (Key*) &barKey);

    // now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
    document.addKey(fooTag, barKey);
    fooKeys = document.getTagKeys(fooTag);
    ASSERT_TRUE(fooKeys.size() == 2);
}

TEST_F(DocumentTests, TestsIfSingleKeyCanBeRemoved) {
    document.addKey(fooTag, fooKey);
    document.addKey(fooTag, barKey);
    KeySet keySet = document.getTagKeys(fooTag);
    ASSERT_TRUE(keySet.size() == 2);

    document.removeKey(fooTag, fooKey);
    keySet = document.getTagKeys(fooTag);
    ASSERT_TRUE(keySet.size() == 1);

    KeySet::iterator it_not_there = keySet.find(&fooKey);
    ASSERT_TRUE(it_not_there == keySet.end());

    KeySet::iterator bar_there = keySet.find(&barKey);
    ASSERT_TRUE(*bar_there == (Key*) &barKey);
}

TEST_F(DocumentTests, TestGetTags) {
    std::string zeeTag = "zeeTag:";
    Key zeeKey(zeeTag);
    document.addKey(zeeTag, zeeKey);
    document.addKey(fooTag, fooKey);
    document.addKey(barTag, barKey);


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
    document.addKey(nameTag, Key(nameTag));

}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}