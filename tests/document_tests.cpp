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
    ;
    document.addKey(fooTag, fooKey);
    document.addKey(fooTag, barKey);

    KeySet fooKeys = document.getTagKeys(fooTag);

    // make sure we have one
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
}

TEST_F(DocumentTests, TestsIfSingleKeyCanBeRemoved) {
    document.addKey(fooTag, fooKey);
    KeySet keySet = document.getTagKeys(fooTag);
    const DocumentKeysMap* docKeys = document.getKeysMap();

    ASSERT_TRUE(docKeys->size() == 1);
    ASSERT_TRUE(keySet.size() == 1);

    document.removeKeys(fooTag);
    ASSERT_TRUE(keySet.size() == 0);
    ASSERT_TRUE(docKeys->size() == 0);
}

TEST_F(DocumentTests, TestTagKeyRemoval) {
    document.addKey(fooTag, fooKey);

}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}