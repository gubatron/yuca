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
// Created by gubatron on 11/9/17.
//

#ifndef YUCA_TYPES_HPP
#define YUCA_TYPES_HPP

#include <memory>
#include <string>
#include <set>
#include "utils.hpp"

namespace yuca {
    class Key;

    class StringKey;

    class Document;

    typedef std::shared_ptr<Key> SPKey;
    typedef std::shared_ptr<StringKey> SPStringKey;
    typedef std::shared_ptr<Document> SPDocument;
    typedef yuca::utils::Set<Key> KeySet;
    typedef yuca::utils::Set<SPKey> SPKeySet;
    typedef yuca::utils::Set<StringKey> StringKeySet;
    typedef yuca::utils::Set<SPStringKey> SPStringKeySet;
    typedef yuca::utils::List<SPKey> SPKeyList;
    typedef yuca::utils::Set<SPDocument> SPDocumentSet;
    typedef yuca::utils::List<SPDocument> SPDocumentList;
}

#endif //YUCA_TYPES_HPP
