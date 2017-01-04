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
        document = Document();
    }

    string fooTag;
    Document document;
    Key fooKey;
};


TEST_F(DocumentTests, TestIfKeyCanBeAdded) {
    document.addKey(fooTag, fooKey);
    KeySet* fooKeys = document.getKeys(fooTag);
    std::cout << "Memory address of the keySet when we do document.getKeys(fooTag) -> " << fooKeys << std::endl;


    // make sure we have one
    ASSERT_TRUE(fooKeys->size() == 1);

    // make sure it's the same one we've added
    KeySet::iterator it = fooKeys->find((Key*)&fooKey);

    //ASSERT_FALSE(it == fooKeys.end());

    Key *isThisFooKeyPtr = *it;
    std::cout << "key found? " << isThisFooKeyPtr->getTag() << std::endl;

    ASSERT_TRUE(isThisFooKeyPtr == (Key*) &fooKey);
//    DocumentKeys::iterator it = docKeys.begin();


}

TEST_F(DocumentTests, TestsIfSingleKeyCanBeRemoved) {
    document.addKey(fooTag, fooKey);
    const DocumentKeys docKeys = document.getKeys();
    ASSERT_TRUE(document.getKeys().size() == 1);

    document.removeKey(fooTag, fooKey);
    ASSERT_TRUE(document.getKeys().size() == 0);
}



int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}