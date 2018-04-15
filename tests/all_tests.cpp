//
// Created by gubatron on 2/22/17.
//

#ifndef YUCA_ALL_TESTS_H
#define YUCA_ALL_TESTS_H

#include <yuca/key.hpp>
#include <yuca/document.hpp>

using namespace yuca;

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

std::string foo_tag(":foo");
std::string bar_tag(":bar");
auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
auto foo_key2_sp = std::make_shared<Key>(2, foo_tag);
auto bar_key_sp = std::make_shared<Key>(1, bar_tag);

std::shared_ptr<Document> document_foo_sp;
std::shared_ptr<Document> document_bar_sp;
std::shared_ptr<Document> document_foo_bar_sp;

#include "utils_tests.cpp"
#include "document_tests.cpp"
#include "indexer_tests.cpp"

#endif //YUCA_ALL_TESTS_H
