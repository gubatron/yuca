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
        foo_tag = ":foo";
        bar_tag = ":bar";
        foo_key_sp = std::make_shared<Key>(1, foo_tag);
        bar_key_sp = std::make_shared<Key>(1, bar_tag);
        bar_key2_sp = std::make_shared<Key>(2, bar_tag);
        document_sp = std::make_shared<Document>();
    }

    std::string foo_tag;
    std::shared_ptr<Key> foo_key_sp;

    std::string bar_tag;
    std::shared_ptr<Key> bar_key_sp;
    std::shared_ptr<Key> bar_key2_sp;
    std::shared_ptr<Document> document_sp;
};


TEST_F(DocumentTests, TestIfKeyCanBeAdded) {
    KeySet is_empty;
    document_sp->getTagKeys(foo_tag, is_empty);
    ASSERT_TRUE(is_empty.empty());

    document_sp->addKey(foo_key_sp);
    KeySet foo_keys;
    document_sp->getTagKeys(foo_tag, foo_keys);
    ASSERT_TRUE(foo_keys.size() == 1);

    if (bar_key_sp.get() < bar_key2_sp.get()) {
        std::cout << "bar_key < bar_key2 indeed" << std::endl;
    } else {
        FAIL();
    }

    document_sp->addKey(bar_key_sp);
    document_sp->addKey(bar_key2_sp);

    KeySet bar_keys;
    document_sp->getTagKeys(bar_tag, bar_keys);
    ASSERT_TRUE(bar_keys.size() == 2);

    // make sure it's the same one we've added

    auto it = foo_keys.find(foo_key_sp);
    if (it == foo_keys.end()) {
        std::cout << "Couldn't find fooKey even though we just added it. What the hell do we have here then?" << std::endl;
        it = foo_keys.begin();
        (*it)->dumpToStream(std::cout);
        std::cout << std::endl << "vs" << std::endl;
        foo_key_sp->dumpToStream(std::cout);
        std::cout << std::endl << "------------------------" << std::endl;
        FAIL();
    }
    ASSERT_TRUE(foo_keys.size() == 1);
    ASSERT_FALSE(it == foo_keys.end());
    ASSERT_TRUE(*it == foo_key_sp);
    ASSERT_TRUE(**it == *foo_key_sp.get());
    ASSERT_TRUE((*it)->getTag() == foo_tag);

    it = bar_keys.find(bar_key_sp);
    Key isThisBarKey = **it;
    std::cout << "tag found? " << isThisBarKey.getTag() << std::endl;
    ASSERT_TRUE(isThisBarKey.getTag() == bar_tag);
    ASSERT_TRUE(isThisBarKey == *bar_key_sp.get());

    it++;
    Key is_this_bar_key2 = **it;
    ASSERT_TRUE(is_this_bar_key2.getTag() == bar_tag);
    ASSERT_TRUE(is_this_bar_key2 == *bar_key2_sp.get());

    // now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
    document_sp->addKey(bar_key_sp);
    document_sp->getTagKeys(bar_tag, bar_keys);
    ASSERT_TRUE(bar_keys.size() == 2);
}

TEST_F(DocumentTests, TestsIfSingleKeyCanBeRemoved) {
    document_sp->addKey(foo_key_sp);
    document_sp->addKey(bar_key_sp);
    KeySet foo_key_set;
    document_sp->getTagKeys(foo_tag, foo_key_set);
    ASSERT_TRUE(foo_key_set.size() == 1);

    KeySet barKeySet;
    document_sp->getTagKeys(bar_tag, barKeySet);
    ASSERT_TRUE(barKeySet.size() == 1);

    document_sp->addKey(bar_key2_sp);
    document_sp->getTagKeys(bar_tag, barKeySet);
    ASSERT_TRUE(barKeySet.size() == 2);

    document_sp->removeKey(foo_tag, foo_key_sp);
    document_sp->getTagKeys(foo_tag, foo_key_set);
    ASSERT_TRUE(foo_key_set.size() == 0);

    KeySet::iterator it_not_there = foo_key_set.find(foo_key_sp);
    ASSERT_TRUE(it_not_there == foo_key_set.end());

    document_sp->getTagKeys(bar_tag, barKeySet);
    KeySet::iterator bar_there = barKeySet.find(bar_key_sp);
    ASSERT_TRUE(**bar_there == *bar_key_sp.get());

    KeySet::iterator bar2_there = barKeySet.find(bar_key2_sp);
    ASSERT_TRUE(bar2_there != barKeySet.end());
    ASSERT_TRUE(**bar2_there == *bar_key2_sp.get());
}

TEST_F(DocumentTests, TestGetTags) {
    std::string zee_tag = ":zee";
    auto zee_key_sp = std::make_shared<Key>(7, zee_tag);

    document_sp->addKey(zee_key_sp);
    document_sp->addKey(foo_key_sp);
    document_sp->addKey(bar_key_sp);

    std::cout << "Getting all the tags...";

    std::set<std::string> tags;
    document_sp->getTags(tags);
    std::cout << "Got " << tags.size() << " tags" << std::endl;
    auto it = tags.begin();
    std::cout << "tags[0] => " << *it++ << std::endl;
    std::cout << "tags[1] => " << *it++ << std::endl;
    std::cout << "tags[2] => " << *it << std::endl;

    // elements will be sorted as they're inserted in the set
    // they are not in the set in the order they were inserted.
    it = tags.begin();
    ASSERT_TRUE(*it++ == bar_tag);
    ASSERT_TRUE(*it++ == foo_tag);
    ASSERT_TRUE(*it == zee_tag);
}

TEST_F(DocumentTests, TestRemoveTag) {
    std::string name_tag = ":name";
    auto name_key_sp = std::make_shared<Key>(4, name_tag);
    document_sp->addKey(name_key_sp);
    document_sp->addKey(bar_key_sp);
    document_sp->addKey(bar_key2_sp);

    KeySet name_tag_keys;
    document_sp->getTagKeys(":name", name_tag_keys);
    KeySet::iterator found_keys = name_tag_keys.find(bar_key_sp);
    ASSERT_TRUE(found_keys == name_tag_keys.end()); //assert not found
    found_keys = name_tag_keys.find(name_key_sp);
    ASSERT_FALSE(found_keys == name_tag_keys.end());
    ASSERT_TRUE(**found_keys == *name_key_sp);
    ASSERT_TRUE(found_keys == name_tag_keys.begin());
    document_sp->dumpToStream(std::cout);
    document_sp->removeKey(name_tag, name_key_sp);
    std::cout << std::endl << "after :name key removed" << std::endl;
    //document_sp->dumpToStream(std::cout);
    document_sp->getTagKeys(":name", name_tag_keys);
    ASSERT_TRUE(name_tag_keys.empty());
    KeySet bar_keys;
    document_sp->getTagKeys(bar_key_sp->getTag(), bar_keys);
    ASSERT_TRUE(bar_keys.size() == 2);
    document_sp->removeTag(bar_key_sp->getTag());
    document_sp->getTagKeys(bar_key_sp->getTag(), bar_keys);
    ASSERT_TRUE(bar_keys.empty());
}

#endif
