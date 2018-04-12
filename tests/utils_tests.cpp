//
// Created by gubatron on 4/11/18.
//

#ifndef YUCA_UTILS_TESTS_H
#define YUCA_UTILS_TESTS_H

TEST_CASE("yuca::utils::Set (add, size, contains, isEmpty)") {
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
}

TEST_CASE("yuca::utils:Set (addAll, containsAll, removeAll)") {
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
#endif