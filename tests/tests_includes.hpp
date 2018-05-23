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
// Created by gubatron on 2/22/17.
//

#ifndef YUCA_ALL_TESTS_H
#define YUCA_ALL_TESTS_H

#include <string>
#include <yuca/utils.hpp>
#include <yuca/types.hpp>
#include <yuca/key.hpp>
#include <yuca/document.hpp>
#include <yuca/indexer.hpp>
#include "catch.hpp"

void initDocumentTests(void);

struct file;

file generateRandomFile(const yuca::utils::List<std::string> title_dict,
                        const yuca::utils::List<std::string> ext_dict,
                        unsigned long min_words,
                        unsigned long max_words);

yuca::utils::List<std::string>
generateRandomPhrase(const yuca::utils::List<std::string> &dictionary, unsigned long words);

// See cmake's yuca_tests to include cpp files with actual tests.
// Such tests all include this file

#endif //YUCA_ALL_TESTS_H
