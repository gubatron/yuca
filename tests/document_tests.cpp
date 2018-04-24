//
// Created by gubatron on 11/19/2016.
//
#ifndef YUCA_DOCUMENT_TESTS_H
#define YUCA_DOCUMENT_TESTS_H

#include "tests_includes.hpp"

extern std::string foo_tag;
extern std::string bar_tag;

void initDocumentTests() {
	foo_tag = std::string(":foo");
	bar_tag = std::string(":bar");
}

TEST_CASE("Test If Key Can Be Added") {
    initDocumentTests();

	auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
	auto bar_key_sp = std::make_shared<Key>(111, bar_tag);
	auto bar_key2_sp = std::make_shared<Key>(222, bar_tag);
	auto document_sp = std::make_shared<Document>();

	KeySet is_empty = document_sp->getTagKeys(foo_tag);
	REQUIRE(is_empty.isEmpty());

	document_sp->addKey(foo_key_sp);
	KeySet foo_keys = document_sp->getTagKeys(foo_tag);
	REQUIRE(foo_keys.size() == 1);

	if (bar_key_sp.get()->getId() < bar_key2_sp.get()->getId()) {
		//std::cout << "bar_key < bar_key2 indeed" << std::endl;
	} else {
		FAIL();
	}

	document_sp->addKey(bar_key_sp);
	document_sp->addKey(bar_key2_sp);

	KeySet bar_keys = document_sp->getTagKeys(bar_tag);
	REQUIRE(bar_keys.size() == 2);

	// make sure it's the same one we've added
	REQUIRE(foo_keys.contains(foo_key_sp));
	REQUIRE(bar_keys.contains(bar_key_sp));
	REQUIRE(bar_keys.contains(bar_key2_sp));

	// now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
	document_sp->addKey(bar_key_sp);
	bar_keys = document_sp->getTagKeys(bar_tag);
	REQUIRE(bar_keys.size() == 2);
}


TEST_CASE("Test if a single Key can be removed") {
	initDocumentTests();

	const auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
	const auto bar_key_sp = std::make_shared<Key>(1, bar_tag);
	const auto bar_key2_sp = std::make_shared<Key>(2, bar_tag);
	const auto document_sp = std::make_shared<Document>();

	document_sp->addKey(foo_key_sp);
	document_sp->addKey(bar_key_sp); // Doc { foo:[Key(1])], bar:[Key(1)] }
	KeySet foo_key_set = document_sp->getTagKeys(foo_tag);
	REQUIRE(foo_key_set.size() == 1);

	KeySet bar_key_set = document_sp->getTagKeys(bar_tag);
	REQUIRE(bar_key_set.size() == 1);

	document_sp->addKey(bar_key2_sp); // Doc { foo:[Key(1)], bar:[Key(1),Key(2)] }
	bar_key_set = document_sp->getTagKeys(bar_tag);
	REQUIRE(bar_key_set.size() == 2);

	foo_key_set = document_sp->getTagKeys(foo_tag);
	REQUIRE(foo_key_set.size() == 1);
	document_sp->removeKey(foo_tag, foo_key_sp);

	// test that yoy need to fetch another copy of the set
	REQUIRE(foo_key_set.size() == 1); // if we don't fetch it's still 1
	REQUIRE(foo_key_set.contains(foo_key_sp));
	REQUIRE(!document_sp->hasKeys(foo_tag)); // but you can ask directly without asking for a set if there are no more
	foo_key_set = document_sp->getTagKeys(foo_tag);

	REQUIRE(!foo_key_set.contains(foo_key_sp));
	REQUIRE(foo_key_set.size() == 0);
	REQUIRE(!foo_key_set.contains(foo_key_sp));

	bar_key_set = document_sp->getTagKeys(bar_tag);
	REQUIRE(bar_key_set.contains(bar_key_sp));
	REQUIRE(bar_key_set.contains(bar_key2_sp));
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

	std::set<std::string> tags = document_sp->getTags();
	auto it = tags.begin();

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

	KeySet name_tag_keys = document_sp->getTagKeys(":name");
	REQUIRE(!name_tag_keys.contains(bar_key_sp));
	REQUIRE(name_tag_keys.contains(name_key_sp));

	document_sp->removeKey(name_tag, name_key_sp);
	name_tag_keys = document_sp->getTagKeys(":name");
	REQUIRE(name_tag_keys.isEmpty());
	KeySet bar_keys;
	bar_keys = document_sp->getTagKeys(bar_key_sp->getTag());
	REQUIRE(bar_keys.size() == 2);
	document_sp->removeTag(bar_key_sp->getTag());
	bar_keys = document_sp->getTagKeys(bar_key_sp->getTag());
	REQUIRE(bar_keys.isEmpty());
}
#endif