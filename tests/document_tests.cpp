//
// Created by gubatron on 11/19/16.
//

#include <gtest/gtest.h>
#include <yuca/document.hpp>

using namespace yuca;

TEST(AddKey, TestIfKeyCanBeAdded) {
    yuca::Document document;
    document.addKey(string("hola"), yuca::Key());
    ASSERT_TRUE(document.getKeys().size() > 10);
}

int main() {
    return 0;
}