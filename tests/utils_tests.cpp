//
// Created by gubatron on 4/11/18.
//

#ifndef YUCA_UTILS_TESTS_H
#define YUCA_UTILS_TESTS_H

#include "tests_includes.hpp"

TEST_CASE("yuca:utils::Set") {
	SECTION("yuca::utils::Set (add, size, contains, isEmpty, clear)") {
		yuca::utils::Set<std::string> set;
		REQUIRE(set.size() == 0);
		REQUIRE(set.isEmpty());

		set.add("one");
		REQUIRE(set.size() == 1);
		REQUIRE(!set.isEmpty());

		REQUIRE(!set.contains("two"));
		REQUIRE(set.contains("one"));

		set.add("two");
		REQUIRE(set.size() == 2);
		REQUIRE(set.contains("two"));

		set.clear();
		REQUIRE(set.size() == 0);
		REQUIRE(set.isEmpty());
	}

	SECTION("yuca::utils::Set (addAll, containsAll, removeAll)") {
		std::string one_tag("one");
		std::string two_tag("two");
		std::string three_tag("three");
		std::string four_tag("four");
		auto k1 = std::make_shared<yuca::Key>(1, one_tag);
		auto k2 = std::make_shared<yuca::Key>(2, two_tag);
		auto k3 = std::make_shared<yuca::Key>(3, three_tag);
		auto k4 = std::make_shared<yuca::Key>(4, four_tag);

		yuca::utils::Set<std::shared_ptr<yuca::Key>> set1;
		yuca::utils::Set<std::shared_ptr<yuca::Key>> set2;

		// Set1 = { k1, k2, k3 }
		// Set2 = { <empty> }
		set1.add(k1);
		set1.add(k2);
		set1.add(k3);
		REQUIRE(set1.size() == 3);
		REQUIRE(set2.isEmpty());

		// Set2 = { k1, k2, k3 }
		set2.addAll(set1);

		REQUIRE(set2.size() == 3);
		REQUIRE(set2.contains(k3));
		REQUIRE(!set2.contains(k4));
		REQUIRE(set2.containsAll(set1));
		REQUIRE(set1.containsAll(set2));

		set2.add(k4);
		// Set2 = { k1, k2, k3, k4 }
		REQUIRE(set2.size() == 4);
		REQUIRE(set2.contains(k4));
		REQUIRE(set2.contains(k1));
		REQUIRE(!set1.containsAll(set2));

		set2.removeAll(set1);
		// Set2 = { k4 }
		REQUIRE(set1.size() == 3);
		REQUIRE(set2.size() == 1);
		REQUIRE(set2.contains(k4));
		REQUIRE(!set2.contains(k3));

		set1.clear();
		// Set1 = {}
		REQUIRE(set1.size() == 0);
		REQUIRE(set1.isEmpty());
	}
}

TEST_CASE("yuca::utils::Map") {
	SECTION("yuca::utils::Map (isEmpty, clear, put, get, remove, keySet)") {
		yuca::utils::Map<std::string, int> m(-1);
		REQUIRE(m.isEmpty());
		m.put("angel", 1);
		REQUIRE(!m.isEmpty());
		int val = m.get("angel");
		REQUIRE(val == 1);
		m.put("angel", m.get("angel") + 1);
		REQUIRE(m.get("angel") == (val + 1));
		int last_val = m.remove("angel");
		REQUIRE(m.isEmpty());
		REQUIRE(last_val == val + 1);
		m.put("desmond", 1);
		m.put("sarah", 2);
		m.put("paulina", 3);
		REQUIRE(m.size() == 3);
		auto key_set = m.keySet();
		REQUIRE(key_set.size() == 3);
		REQUIRE(key_set.contains("desmond"));
		REQUIRE(key_set.contains("sarah"));
		REQUIRE(key_set.contains("paulina"));
		REQUIRE(!key_set.contains("angel"));
		m.clear();
		key_set = m.keySet();
		REQUIRE(key_set.isEmpty());
	}

	SECTION("yuca::utils::Map tests with shared pointers") {
		yuca::utils::Map<std::shared_ptr<StringKey>, std::shared_ptr<Document>> m(nullptr);
		std::string foo("foo");
		std::string tag(":tag");
		auto foo_key_sp = std::make_shared<StringKey>(foo,tag);
		auto doc = std::make_shared<Document>();
		doc->addKey(foo_key_sp);

		m.put(foo_key_sp, doc);
		REQUIRE(m.size() == 1);
		REQUIRE(m.containsKey(foo_key_sp));
		REQUIRE(m.get(foo_key_sp) == doc);

		// copies of shared_ptr keys can't be found
		std::string foo_prime("foo");
		std::string tag_prime(":tag");
		auto foo_prime_key_sp = std::make_shared<StringKey>(foo_prime, tag_prime);
		REQUIRE(!m.containsKey(foo_prime_key_sp));
		REQUIRE(m.get(foo_prime_key_sp) != doc);


		yuca::utils::Map<StringKey, std::shared_ptr<Document>> m2(nullptr);
		StringKey foo_key(foo, tag);
		StringKey foo_prime_key(foo, tag);
		auto doc2 = std::make_shared<Document>();
		doc2->addKey(std::make_shared<StringKey>(foo_key));
		m2.put(foo_key, doc2);
		REQUIRE(m2.containsKey(foo_key));
		REQUIRE(m2.get(foo_key) == doc2);
		REQUIRE(m2.containsKey(foo_prime_key));
		REQUIRE(m2.get(foo_prime_key) == doc2);
	}

	SECTION("yuca::utils::Map (putAll, entrySet)") {
		yuca::utils::Map<std::string, std::string> m1("");
		yuca::utils::Map<std::string, std::string> m2("");

		m1.put("1", "one");
		m1.put("2", "two");
		m1.put("3", "three");

		REQUIRE(m1.size() == 3);
		REQUIRE(m2.size() == 0);

		REQUIRE(m2.get("1") == "");
		REQUIRE(m2.size() == 0); // failed query should not increase container size

		m2.putAll(m1);
		REQUIRE(m2.size() == 3);
		REQUIRE(m2.containsKey("3"));
		REQUIRE(m2.containsKey("2"));
		REQUIRE(m2.containsKey("1"));

		m2.put("4", "four");
		m2.put("5", "five");
		m2.put("6", "six");

		m1.putAll(m2);

		REQUIRE(m2.size() == 6);
		REQUIRE(m1.size() == 6);

		auto m2_keyset = m2.keySet().getStdSet();
		for (auto const &k : m2_keyset) {
			REQUIRE(m1.containsKey(k));
			REQUIRE(m1.get(k) == m2.get(k));
		}

		auto m2_keylist = m2.keyList();
		for (int i = 0; i < m2_keylist.size(); i++) {
			auto k = m2_keylist.get(i);
			REQUIRE(m1.containsKey(k));
			REQUIRE(m1.get(k) == m2.get(k));
		}
	}
}

TEST_CASE(
"yuca::utils::List  isEmpty, size, add, add(i,t), addAll(List), addAll(Set), indexOf, contains, get, removeAt, removeAll") {
	yuca::utils::List<int> intList;
	REQUIRE(intList.size() == 0);
	REQUIRE(intList.isEmpty());

	intList.add(1);
	REQUIRE(intList.size() == 1);
	REQUIRE(!intList.isEmpty());

	intList.add(2);
	REQUIRE(intList.size() == 2);

	intList.add(4);
	REQUIRE(intList.size() == 3);

	REQUIRE(intList.contains(1));
	REQUIRE(intList.contains(2));
	REQUIRE(intList.contains(4));
	REQUIRE(!intList.contains(3));

	REQUIRE(intList.indexOf(1) == 0);
	REQUIRE(intList.indexOf(2) == 1);
	REQUIRE(intList.indexOf(3) == -1);
	REQUIRE(intList.indexOf(4) == 2);

	intList.add(2, 3);
	REQUIRE(intList.size() == 4);
	REQUIRE(intList.contains(1));
	REQUIRE(intList.contains(2));
	REQUIRE(intList.contains(3));
	REQUIRE(intList.contains(4));
	REQUIRE(intList.indexOf(1) == 0);
	REQUIRE(intList.indexOf(2) == 1);
	REQUIRE(intList.indexOf(3) == 2);
	REQUIRE(intList.indexOf(4) == 3);

	yuca::utils::List<int> emptyList;
	yuca::utils::List<int> other3;
	other3.add(5);
	other3.add(6);
	other3.add(7);

	// empty list doesn't change intList
	intList.addAll(emptyList);
	REQUIRE(intList.size() == 4);

	intList.addAll(other3);
	REQUIRE(intList.size() == 7);
	REQUIRE(intList.indexOf(5) == 4);
	REQUIRE(intList.indexOf(6) == 5);
	REQUIRE(intList.indexOf(7) == 6);

	REQUIRE(intList.get(0) == 1);
	REQUIRE(intList.get(1) == 2);
	REQUIRE(intList.get(2) == 3);
	REQUIRE(intList.get(3) == 4);
	REQUIRE(intList.get(4) == 5);
	REQUIRE(intList.get(5) == 6);
	REQUIRE(intList.get(6) == 7);

	// empty list doesn't change intList
	REQUIRE(!intList.removeAll(emptyList));

	REQUIRE(intList.removeAll(other3));
	REQUIRE(intList.size() == 4);

	intList.removeAll(other3);
	REQUIRE(intList.contains(1));
	REQUIRE(intList.contains(2));
	REQUIRE(intList.contains(3));
	REQUIRE(intList.contains(4));
	REQUIRE(!intList.contains(5));
	REQUIRE(!intList.contains(6));
	REQUIRE(!intList.contains(7));

	REQUIRE(!intList.remove(5));
	REQUIRE(intList.remove(3));
	REQUIRE(intList.remove(1));
	REQUIRE(!intList.contains(1));
	REQUIRE(intList.contains(2));
	REQUIRE(!intList.contains(3));
	REQUIRE(intList.contains(4));
	REQUIRE(intList.size() == 2);

	yuca::utils::Set<int> intSet;
	intSet.add(100);
	intSet.add(200);
	intSet.add(300);

	intList.addAll(intSet);
	REQUIRE(intList.size() == 5);
	REQUIRE(intList.get(0) == 2);
	REQUIRE(intList.get(1) == 4);
	REQUIRE(intList.get(2) == 100);
	REQUIRE(intList.get(3) == 200);
	REQUIRE(intList.get(4) == 300);
	REQUIRE(intList.indexOf(300) == 4);
	intList.set(4, 333);
	REQUIRE(!intList.contains(300));
	REQUIRE(intList.contains(333));
	REQUIRE(intList.indexOf(333) == 4);

	REQUIRE(intList.removeAt(2));
	REQUIRE(intList.size() == 4);
	REQUIRE(!intList.contains(100));
	REQUIRE(intList.contains(200));
	REQUIRE(intList.contains(333));
	intList.clear();
	REQUIRE(intList.size() == 0);
	REQUIRE(!intList.contains(333));

	yuca::utils::List<yuca::Key> keyList;
	std::string title_tag(":title");
	Key k1(1, title_tag);
	keyList.add(k1);
	keyList.add(k1);
	keyList.add(k1);
	REQUIRE(keyList.size() == 3);
	keyList.remove(k1);
	REQUIRE(keyList.size() == 2);
	keyList.remove(k1);
	REQUIRE(keyList.size() == 1);

	yuca::utils::List<std::shared_ptr<Key>> spKeyList;
	auto sp_k1 = std::make_shared<Key>(k1);
	auto sp_k2 = std::make_shared<Key>(Key(2, title_tag));
	spKeyList.add(sp_k1);
	spKeyList.add(sp_k2);
	REQUIRE(spKeyList.indexOf(sp_k1) == 0);
	REQUIRE(spKeyList.indexOf(sp_k2) == 1);
	REQUIRE(spKeyList.get(0) == sp_k1);
	REQUIRE(spKeyList.get(1) == sp_k2);

	auto sp_k2_prime = spKeyList.get(1);
	REQUIRE(sp_k2 == sp_k2_prime);
}

TEST_CASE("yuca:utils static functions") {
	SECTION("yuca::utils split") {
		std::string foo_query = "one two three four";
		auto list = yuca::utils::split(foo_query);
		REQUIRE(list.size() == 4);
		REQUIRE(list.get(0) == "one");
		REQUIRE(list.get(1) == "two");
		REQUIRE(list.get(2) == "three");
		REQUIRE(list.get(3) == "four");
	}
}

#endif