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
// Created by Alden Torres, gubatron on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include <map>
#include <set>
#include <vector>
#include <chrono>
#include "key.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace yuca {
    enum PropertyType {
        BOOL,
        BYTE,
        INT,
        LONG,
        STRING
    };

    class Document {
    public:
        explicit Document(long doc_id) :
        id(doc_id),
        tag_2_keyset_map(SPKeySet()),
        bool_properties(false),
        byte_properties(0),
        int_properties(-1),
        long_properties(-1l),
        string_properties("")
        {
        }

        explicit Document(const std::string &str_based_id) : Document(static_cast<long>(std::hash<std::string>{}(str_based_id))) {}

        long getId() const;

        /** Associate this document to an indexing key under the given tag */
        void addKey(Key const &key);

        /** Associate this document to an indexing key under the given tag */
        void addKey(std::shared_ptr<Key> key);

        /** Does this document have at least one key under this tag? */
        bool hasKeys(std::string const &tag) const;

        /** Returns a copy of all tags under which we have KeySets */
        std::set<std::string> getTags() const;

        /** Returns a copy of all keys shared pointers available under a given tag */
        SPKeySet getTagSPKeys(std::string const &tag) const;

        KeySet getTagKeys(std::string const &tag) const;

        /** Removes all keys under this tag */
        void removeTag(std::string const &tag);

        /** Removes the given key. If it's the last key for this tag, the tag itself is removed */
        void removeKey(std::string const &tag, std::shared_ptr<Key> key);

        /** Set the value of a bool property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void boolProperty(std::string const &key, bool value);

        /** Gets the value of a bool property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        bool boolProperty(std::string const &key) const;

        /** Removes a a bool property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void removeBoolProperty(std::string const &key);

        /** Set the value of a byte property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void byteProperty(std::string const &key, char value);

        /** Gets the value of a byte property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        char byteProperty(std::string const &key) const;

        /** Removes a a byte property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void removeByteProperty(std::string const &key);

        /** Set the value of a int property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void intProperty(std::string const &key, int value);

        /** Gets the value of a int property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        int intProperty(std::string const &key) const;

        /** Removes a a int property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void removeIntProperty(std::string const &key);

        /** Set the value of a long property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void longProperty(std::string const &key, long value);

        /** Gets the value of a long property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        long longProperty(std::string const &key) const;

        /** Removes a a long property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void removeLongProperty(std::string const &key);

        /** Set the value of a string property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void stringProperty(std::string const &key, std::string const &value);

        /** Gets the value of a string property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        std::string stringProperty(std::string const &key) const;

        /** Removes a a string property under the given key.
         * Properties are not indexed for search, they are meant to be used
         * as Document attributes once they appear as relevant search results */
        void removeStringProperty(std::string const &key);

        /** Retrieves the list of available Document properties under a given PropertyType */
        yuca::utils::List<std::string> propertyKeys(PropertyType type) const;

        bool operator<(const Document &other) const;

        bool operator==(const Document &other) const;

        Document& operator=(const Document& d);

        friend std::ostream& operator<<(std::ostream &output_stream, const Document &doc);

        static const Document NULL_DOCUMENT;

    private:
        const long id;

        // maps tags to set<Key>
        yuca::utils::Map<std::string, SPKeySet> tag_2_keyset_map;

        // docs can have unique properties that are not necessarily indexed keys
        // but which are meant to be retrieved once they appear in search results
        yuca::utils::Map<std::string, bool> bool_properties;
        yuca::utils::Map<std::string, char> byte_properties;
        yuca::utils::Map<std::string, int> int_properties;
        yuca::utils::Map<std::string, long> long_properties;
        yuca::utils::Map<std::string, std::string> string_properties;
    };
}

#endif //YUCA_DOCUMENT_HPP
