//
// Created by gubatron on 11/19/2016.
//
#ifndef YUCA_DOCUMENT_TESTS_H
#define YUCA_DOCUMENT_TESTS_H

TEST_CASE("Test If Key Can Be Added") {
    auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
    auto bar_key_sp = std::make_shared<Key>(111, bar_tag);
    auto bar_key2_sp = std::make_shared<Key>(222, bar_tag);
    bar_key_sp->dumpToStream(std::cout);
    std::cout << std::endl;
    bar_key2_sp->dumpToStream(std::cout);
    std::cout << std::endl;
    auto document_sp = std::make_shared<Document>();

    KeySet is_empty;
    document_sp->getTagKeys(foo_tag, is_empty);
    REQUIRE(is_empty.empty());

    document_sp->addKey(foo_key_sp);
    KeySet foo_keys;
    document_sp->getTagKeys(foo_tag, foo_keys);
    REQUIRE(foo_keys.size() == 1);

    if (bar_key_sp.get()->getId() < bar_key2_sp.get()->getId()) {
        //std::cout << "bar_key < bar_key2 indeed" << std::endl;
    } else {
        FAIL();
    }

    document_sp->addKey(bar_key_sp);
    document_sp->addKey(bar_key2_sp);

    KeySet bar_keys;
    document_sp->getTagKeys(bar_tag, bar_keys);
    REQUIRE(bar_keys.size() == 2);

    // make sure it's the same one we've added

    auto it = foo_keys.find(foo_key_sp);
    REQUIRE(it != foo_keys.end());
    REQUIRE(*it == foo_key_sp);
    REQUIRE(**it == *foo_key_sp.get());
    REQUIRE((*it)->getTag() == foo_tag);
    it++;
    REQUIRE(it == foo_keys.end());

    it = bar_keys.find(bar_key_sp);
    Key is_this_bar_key = *it->get();
    std::cout << std::endl;
    is_this_bar_key.dumpToStream(std::cout);
    std::cout << std::endl;
    bar_key_sp.get()->dumpToStream(std::cout);
    //std::cout << "tag found? " << isThisBarKey.getTag() << std::endl;
    REQUIRE(is_this_bar_key.getTag() == bar_tag);
    REQUIRE(is_this_bar_key.getId() == bar_key_sp.get()->getId());
    REQUIRE(is_this_bar_key == *bar_key_sp.get());

    it = bar_keys.find(bar_key2_sp);
    Key is_this_bar_key2 = *it->get();
    std::cout << std::endl;
    is_this_bar_key2.dumpToStream(std::cout);
    REQUIRE(is_this_bar_key2.getTag() == bar_tag);
    REQUIRE(bar_key2_sp.get()->getId() == is_this_bar_key2.getId());
    REQUIRE(is_this_bar_key2 == *bar_key2_sp.get());

    // now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
    document_sp->addKey(bar_key_sp);
    document_sp->getTagKeys(bar_tag, bar_keys);
    REQUIRE(bar_keys.size() == 2);
}


TEST_CASE("Test if a single Key can be removed") {
    auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
    auto bar_key_sp = std::make_shared<Key>(1, bar_tag);
    auto bar_key2_sp = std::make_shared<Key>(2, bar_tag);
    auto document_sp = std::make_shared<Document>();

    document_sp->addKey(foo_key_sp);
    document_sp->addKey(bar_key_sp);
    KeySet foo_key_set;
    document_sp->getTagKeys(foo_tag, foo_key_set);
    REQUIRE(foo_key_set.size() == 1);

    KeySet barKeySet;
    document_sp->getTagKeys(bar_tag, barKeySet);
    REQUIRE(barKeySet.size() == 1);

    document_sp->addKey(bar_key2_sp);
    document_sp->getTagKeys(bar_tag, barKeySet);
    REQUIRE(barKeySet.size() == 2);

    document_sp->removeKey(foo_tag, foo_key_sp);
    REQUIRE(foo_key_set.size() == 1);
    document_sp->getTagKeys(foo_tag, foo_key_set);
    REQUIRE(foo_key_set.size() == 0);

    KeySet::iterator it_not_there = foo_key_set.find(foo_key_sp);
    REQUIRE(it_not_there == foo_key_set.end());

    document_sp->getTagKeys(bar_tag, barKeySet);
    KeySet::iterator bar_there = barKeySet.find(bar_key_sp);
    REQUIRE(**bar_there == *bar_key_sp.get());

    KeySet::iterator bar2_there = barKeySet.find(bar_key2_sp);
    REQUIRE(false == (bar2_there == barKeySet.end()));
    REQUIRE(**bar2_there == *bar_key2_sp.get());
}

TEST_CASE("Test Get Tags") {
    auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
    auto bar_key_sp = std::make_shared<Key>(1, bar_tag);
    auto bar_key2_sp = std::make_shared<Key>(2, bar_tag);
    auto document_sp = std::make_shared<Document>();

    std::string zee_tag = ":zee";
    auto zee_key_sp = std::make_shared<Key>(7, zee_tag);

    document_sp->addKey(zee_key_sp);
    document_sp->addKey(foo_key_sp);
    document_sp->addKey(bar_key_sp);

    //std::cout << "Getting all the tags...";

    std::set<std::string> tags = document_sp->getTags();
    //std::cout << "Got " << tags.size() << " tags" << std::endl;
    auto it = tags.begin();
//    std::cout << "tags[0] => " << *it++ << std::endl;
//    std::cout << "tags[1] => " << *it++ << std::endl;
//    std::cout << "tags[2] => " << *it << std::endl;

    // elements will be sorted as they're inserted in the set
    // they are not in the set in the order they were inserted.
    it = tags.begin();
    REQUIRE(*it++ == bar_tag);
    REQUIRE(*it++ == foo_tag);
    REQUIRE(*it == zee_tag);
}

TEST_CASE("Test Removing a Tag") {
    std::string name_tag = ":name";
    auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
    auto bar_key_sp = std::make_shared<Key>(1, bar_tag);
    auto bar_key2_sp = std::make_shared<Key>(2, bar_tag);
    auto document_sp = std::make_shared<Document>();

    auto name_key_sp = std::make_shared<Key>(4, name_tag);
    document_sp->addKey(name_key_sp);
    document_sp->addKey(bar_key_sp);
    document_sp->addKey(bar_key2_sp);

    KeySet name_tag_keys;
    document_sp->getTagKeys(":name", name_tag_keys);
    KeySet::iterator found_keys = name_tag_keys.find(bar_key_sp);
    REQUIRE(found_keys == name_tag_keys.end()); //assert not found
    found_keys = name_tag_keys.find(name_key_sp);
    REQUIRE(found_keys != name_tag_keys.end());
    REQUIRE(**found_keys == *name_key_sp);
    REQUIRE(found_keys == name_tag_keys.begin());
    //document_sp->dumpToStream(std::cout);
    document_sp->removeKey(name_tag, name_key_sp);
    //std::cout << std::endl << "after :name key removed" << std::endl;
    //document_sp->dumpToStream(std::cout);
    document_sp->getTagKeys(":name", name_tag_keys);
    REQUIRE(name_tag_keys.empty());
    KeySet bar_keys;
    document_sp->getTagKeys(bar_key_sp->getTag(), bar_keys);
    REQUIRE(bar_keys.size() == 2);
    document_sp->removeTag(bar_key_sp->getTag());
    document_sp->getTagKeys(bar_key_sp->getTag(), bar_keys);
    REQUIRE(bar_keys.empty());
}

#endif
