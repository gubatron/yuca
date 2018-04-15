//
// Created by gubatron on 4/11/18.
//

#ifndef YUCA_UTILS_TESTS_H
#define YUCA_UTILS_TESTS_H

TEST_CASE("yuca::utils::Set (add, size, contains, isEmpty, clear)") {
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

TEST_CASE("yuca::utils::Set (addAll, containsAll, removeAll)") {
	std::string one_tag("one");
	std::string two_tag("two");
	std::string three_tag("three");
	std::string four_tag("four");
	auto k1 = std::make_shared<yuca::Key>(1, one_tag);
	auto k2 = std::make_shared<yuca::Key>(2, two_tag);
	auto k3 = std::make_shared<yuca::Key>(3, three_tag);
	auto k4 = std::make_shared<yuca::Key>(4, four_tag);

	yuca::utils::Set<std::shared_ptr<Key>> set1;
	yuca::utils::Set<std::shared_ptr<Key>> set2;

	// Set1 = { k1, k2, k3 }
	// Set2 = { <empty> }
	set1.add(k1); set1.add(k2); set1.add(k3);
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

TEST_CASE("yuca::utils::Map (isEmpty, clear, put, get, remove, keySet)") {
	yuca::utils::Map<std::string, int> m(-1);
	REQUIRE(m.isEmpty());
	m.put("angel",1);
	REQUIRE(!m.isEmpty());
    int val = m.get("angel");
    REQUIRE(val == 1);
    m.put("angel", m.get("angel")+1);
    REQUIRE(m.get("angel") == (val+1));
    int last_val = m.remove("angel");
    REQUIRE(m.isEmpty());
    REQUIRE(last_val == val+1); // 2
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

TEST_CASE("yuca::utils::Map (putAll, entrySet)") {
	yuca::utils::Map<std::string, std::string> m1("");
	yuca::utils::Map<std::string, std::string> m2("");

	m1.put("1","one");
	m1.put("2","two");
	m1.put("3","three");

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
	for (auto const& k : m2_keyset) {
		REQUIRE(m1.containsKey(k));
		REQUIRE(m1.get(k) == m2.get(k));
	}
}

#endif