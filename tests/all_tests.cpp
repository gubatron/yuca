//
// Created by gubatron on 2/22/17.
//

#ifndef YUCA_ALL_TESTS_H
#define YUCA_ALL_TESTS_H

#include <gtest/gtest.h>
#include "document_tests.cpp"
#include "indexer_tests.cpp"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif //YUCA_ALL_TESTS_H
