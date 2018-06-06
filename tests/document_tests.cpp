/**
 * MIT License
 *
 * Copyright (c) 2016-2018 Angel Leon, Alden Torres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// Created by gubatron on 11/19/2016.
//

#include "tests_includes.hpp"

using namespace yuca;

TEST_CASE("Test If Key Can Be Added") {
	std::string foo_group(":foo");
	std::string bar_group(":bar");

	auto foo_key_sp = std::make_shared<StringKey>("foo 1", foo_group);
	auto bar_key_sp = std::make_shared<StringKey>("bar key 1", bar_group);
	auto bar_key2_sp = std::make_shared<StringKey>("bar key", bar_group);
	auto document_sp = std::make_shared<Document>("some_doc_id");

	SPStringKeySet is_empty = document_sp->getGroupSPKeys(foo_group);
	REQUIRE(is_empty.isEmpty());

	document_sp->addKey(foo_key_sp);
	SPStringKeySet foo_keys = document_sp->getGroupSPKeys(foo_group);
	REQUIRE(foo_keys.size() == 1);

	REQUIRE(bar_key_sp.get()->getId() < bar_key2_sp.get()->getId());

	document_sp->addKey(bar_key_sp);
	document_sp->addKey(bar_key2_sp);
	//std::cout << *document_sp << std::endl;

	SPStringKeySet bar_keys = document_sp->getGroupSPKeys(bar_group);
	REQUIRE(bar_keys.size() == 2);

	// make sure it's the same one we've added
	//REQUIRE(foo_keys.contains(std::dynamic_pointer_cast<Key,StringKey>(foo_key_sp)));
	REQUIRE(foo_keys.contains(foo_key_sp));
	REQUIRE(bar_keys.contains(bar_key_sp));
	REQUIRE(bar_keys.contains(bar_key2_sp));

	// now let's try adding the same element again, size should be the same as we're dealing with a set of unique keys
	document_sp->addKey(bar_key_sp);
	bar_keys = document_sp->getGroupSPKeys(bar_group);
	REQUIRE(bar_keys.size() == 2);
}

TEST_CASE("Test with non shared pointer argument methods") {
	Document doc("test_doc_id");
	std::string group(":group");
	StringKey sKey("foo", group);
	doc.addKey(sKey);

	REQUIRE(doc.hasKeys(":group"));

	auto spKeys = doc.getGroupSPKeys(group);
	REQUIRE(spKeys.size() == 1);

	// yes, this is meant to do just one iteration.
	for (auto const& spKey : spKeys.getStdSet()) {
	  auto first_key = *spKey;
	  REQUIRE(first_key == sKey);
	  if (first_key == sKey) {
		  break;
	  }
	}
}


TEST_CASE("Test if a single Key can be removed") {
	std::string foo_group(":foo");
	std::string bar_group(":bar");

	const auto foo_key_sp = std::make_shared<StringKey>("foo 1", foo_group);
	const auto bar_key_sp = std::make_shared<StringKey>("bar 1", bar_group);
	const auto bar_key2_sp = std::make_shared<StringKey>("bar 2", bar_group);
	const auto document_sp = std::make_shared<Document>("doc_id");

	document_sp->addKey(foo_key_sp);
	document_sp->addKey(bar_key_sp); // Doc { foo:[Key(1])], bar:[Key(1)] }
	SPStringKeySet foo_key_set = document_sp->getGroupSPKeys(foo_group);
	REQUIRE(foo_key_set.size() == 1);

	SPStringKeySet bar_key_set = document_sp->getGroupSPKeys(bar_group);
	REQUIRE(bar_key_set.size() == 1);

	document_sp->addKey(bar_key2_sp); // Doc { foo:[Key(1)], bar:[Key(1),Key(2)] }
	bar_key_set = document_sp->getGroupSPKeys(bar_group);
	REQUIRE(bar_key_set.size() == 2);

	foo_key_set = document_sp->getGroupSPKeys(foo_group);
	REQUIRE(foo_key_set.size() == 1);
	document_sp->removeKey(foo_group, foo_key_sp);

	// test that yoy need to fetch another copy of the set
	REQUIRE(foo_key_set.size() == 1); // if we don't fetch it's still 1
	REQUIRE(foo_key_set.contains(foo_key_sp));
	REQUIRE(!document_sp->hasKeys(foo_group)); // but you can ask directly without asking for a set if there are no more
	foo_key_set = document_sp->getGroupSPKeys(foo_group);

	REQUIRE(!foo_key_set.contains(foo_key_sp));
	REQUIRE(foo_key_set.size() == 0);
	REQUIRE(!foo_key_set.contains(foo_key_sp));

	bar_key_set = document_sp->getGroupSPKeys(bar_group);
	REQUIRE(bar_key_set.contains(bar_key_sp));
	REQUIRE(bar_key_set.contains(bar_key2_sp));
}

TEST_CASE("Test Get Groups") {
	std::string foo_group(":foo");
	std::string bar_group(":bar");
	auto foo_key_sp = std::make_shared<StringKey>("foo key", foo_group);
	auto bar_key_sp = std::make_shared<StringKey>("bar key", bar_group);
	auto bar_key2_sp = std::make_shared<StringKey>("bar key 2", bar_group);
	auto document_sp = std::make_shared<Document>("doc_id");

	std::string zee_group = ":zee";
	auto zee_key_sp = std::make_shared<StringKey>("zee key", zee_group);

	document_sp->addKey(zee_key_sp);
	document_sp->addKey(foo_key_sp);
	document_sp->addKey(bar_key_sp);

	std::set<std::string> groups = document_sp->getGroups();
	auto it = groups.begin();

	// elements will be sorted as they're inserted in the set
	// they are not in the set in the order they were inserted.
	REQUIRE(*it++ == bar_group);
	REQUIRE(*it++ == foo_group);
	REQUIRE(*it == zee_group);
}

TEST_CASE("Test Removing a Group") {
	std::string foo_group(":foo");
	std::string bar_group(":bar");
	std::string name_group = ":name";
	auto foo_key_sp = std::make_shared<StringKey>("foo key", foo_group);
	auto bar_key_sp = std::make_shared<StringKey>("bar key", bar_group);
	auto bar_key2_sp = std::make_shared<StringKey>("ba key2", bar_group);
	auto document_sp = std::make_shared<Document>("doc_id");

	auto name_key_sp = std::make_shared<StringKey>("name key", name_group);
	document_sp->addKey(name_key_sp);
	document_sp->addKey(bar_key_sp);
	document_sp->addKey(bar_key2_sp);

	SPStringKeySet name_group_keys = document_sp->getGroupSPKeys(":name");
	REQUIRE(!name_group_keys.contains(bar_key_sp));
	REQUIRE(name_group_keys.contains(name_key_sp));

	document_sp->removeKey(name_group, name_key_sp);
	name_group_keys = document_sp->getGroupSPKeys(":name");
	REQUIRE(name_group_keys.isEmpty());
	SPStringKeySet bar_keys;
	bar_keys = document_sp->getGroupSPKeys(bar_key_sp->getGroup());
	REQUIRE(bar_keys.size() == 2);
	document_sp->removeGroup(bar_key_sp->getGroup());
	bar_keys = document_sp->getGroupSPKeys(bar_key_sp->getGroup());
	REQUIRE(bar_keys.isEmpty());
}

TEST_CASE("Document properties tests") {
	Document doc("doc1");
	Document doc2("doc2");
	doc.stringProperty("file_name", "foo.txt");
	doc2.stringProperty("file_name", "bar.txt");

	SECTION("Document bool properties tests") {
		doc.boolProperty("is_cool", true);
		doc.boolProperty("is_directory", false);
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 2);
		REQUIRE(!doc.boolProperty("is_directory"));
		REQUIRE(doc.boolProperty("is_cool"));
		REQUIRE(doc.boolProperty("unset_property") == false);
		doc.removeBoolProperty("is_cool");
		REQUIRE(!doc.boolProperty("is_cool"));
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 1);
	}

	SECTION("Document byte properties tests") {
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 1);
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).size() == 0);

		doc.byteProperty("merkle_root_prefix", 'A');
		REQUIRE(doc.byteProperty("merkle_root_prefix") ==  65);
		REQUIRE(doc.byteProperty("unknown_prop") == 0);
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).size() == 1);
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).get(0) == "merkle_root_prefix");
		doc.byteProperty("merkle_root_prefix", 2);
		REQUIRE(doc.byteProperty("merkle_root_prefix") != 65);
		REQUIRE(doc.byteProperty("merkle_root_prefix") ==  2);
		doc.removeByteProperty("merkle_root_prefix");
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).size() == 0);
		REQUIRE(doc.byteProperty("merkle_root_prefix") ==  0);
	}

	SECTION("Document int properties tests") {
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 1);
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::INT).size() == 0);
		doc.intProperty("modes", 777);
		REQUIRE(doc.intProperty("modes") == 777);
		doc.intProperty("modes", 1 + doc.intProperty("modes"));
		REQUIRE(doc.intProperty("modes") == 778);
		doc.intProperty("modes", -777);
		REQUIRE(doc.intProperty("modes") == -777);
		doc.removeIntProperty("modes");
		REQUIRE(doc.intProperty("modes") == -1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"
		doc.intProperty("wont_fit", 6301973179050681573); // (ON PURPOSE) warning: implicit conversion from 'long' to 'int'
#pragma clang diagnostic pop
		REQUIRE(doc.intProperty("wont_fit") == -1856314139);
	}

	SECTION("Document long properties tests") {
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 1);
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::INT).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::LONG).size() == 0);
		doc.longProperty("size", 6301973179050681573l);
		REQUIRE(doc.longProperty("size") == 6301973179050681573l);
		doc.longProperty("size", 1 + doc.longProperty("size"));
		REQUIRE(doc.longProperty("size") == 6301973179050681574l);
		doc.longProperty("size", -6301973179050681574);
		REQUIRE(doc.longProperty("size") == -6301973179050681574l);
		doc.removeLongProperty("size");
		REQUIRE(doc.longProperty("size") == -1);
	}

	SECTION("Document string props tests") {
		// TEST CASE runs from start when section starts
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 1);

		REQUIRE(doc.stringProperty("file_name") == "foo.txt");
		REQUIRE(doc.stringProperty("file_ext") == "");

		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 1);
		REQUIRE(doc.propertyKeys(PropertyType::BOOL).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::BYTE).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::INT).size() == 0);
		REQUIRE(doc.propertyKeys(PropertyType::LONG).size() == 0);

		doc.stringProperty("file_ext", "txt");
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 2);

		for (auto const &prop_key : doc.propertyKeys(PropertyType::STRING).getStdVector()) {
			if (prop_key == "file_name") {
				REQUIRE(doc.stringProperty(prop_key) == "foo.txt");
			} else if (prop_key == "file_ext") {
				REQUIRE(doc.stringProperty(prop_key) == "txt");
			}
		}

		doc.removeStringProperty("file_name");
		REQUIRE(doc.stringProperty("file_name") == "");
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 1);
		REQUIRE(doc2.propertyKeys(PropertyType::STRING).size() == 1);
		doc.removeStringProperty("file_ext");
		REQUIRE(doc.propertyKeys(PropertyType::STRING).size() == 0);
		REQUIRE(doc2.propertyKeys(PropertyType::STRING).size() == 1);
		REQUIRE(doc2.stringProperty("file_name") == "bar.txt");
		doc.removeStringProperty("garbage");
	}
}

