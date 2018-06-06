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

#include <iostream>
#include "document.hpp"

namespace yuca {
    const Document Document::NULL_DOCUMENT(-1);

    std::set<std::string> Document::getGroups() const {
        std::set<std::string> groups_out;
        if (group_2_keyset_map.isEmpty()) {
            return groups_out;
        }
        return group_2_keyset_map.keySet().getStdSet();
    }

    long Document::getId() const {
        return id;
    }

    StringKeySet Document::getGroupKeys(std::string const &group) const {
        StringKeySet keySet;
        SPStringKeySet spKeySet = getGroupSPKeys(group);
        for (auto const &spKey : spKeySet.getStdSet()) {
            keySet.add(*spKey);
        }
        return keySet;
    }

    SPStringKeySet Document::getGroupSPKeys(std::string const &group) const {
        if (!hasKeys(group)) {
            return SPStringKeySet();
        }
        return group_2_keyset_map.get(group);
    }

    void Document::addKey(StringKey const &key) {
        auto spKey = std::make_shared<StringKey>(key);
        addKey(std::move(spKey));
    }

    void Document::addKey(SPStringKey key) {
        std::string group(key->getGroup());
        if (!hasKeys(group)) {
            group_2_keyset_map.put(group, SPStringKeySet());
        }
        SPStringKeySet key_set = group_2_keyset_map.get(group);
        key_set.add(key);
        group_2_keyset_map.put(group, key_set);
    }

    void Document::removeKey(std::string const &group, SPStringKey key) {
        if (!hasKeys(group)) {
            return;
        }
        SPStringKeySet keys = getGroupSPKeys(group);
        keys.remove(std::move(key));
        group_2_keyset_map.put(group, keys);

        // once we know the keySet has been cleared we remove it altogether from our { string -> [key0, key1] } map.
        keys = getGroupSPKeys(group);
        if (keys.isEmpty()) {
            removeGroup(group);
        }
    }

    bool Document::hasKeys(std::string const &group) const {
        return group_2_keyset_map.containsKey(group);
    }

    void Document::removeGroup(std::string const &group) {
        if (!hasKeys(group)) {
            return;
        }
        group_2_keyset_map.get(group).clear();
        group_2_keyset_map.remove(group);
    }

    void Document::boolProperty(std::string const &key, bool value) {
        bool_properties.put(key, value);
    }

    bool Document::boolProperty(std::string const &key) const {
        return bool_properties.get(key);
    }

    void Document::removeBoolProperty(std::string const &key) {
        bool_properties.remove(key);
    }

    void Document::byteProperty(std::string const &key, char value) {
        byte_properties.put(key, value);
    }

    char Document::byteProperty(std::string const &key) const {
        return byte_properties.get(key);
    }

    void Document::removeByteProperty(std::string const &key) {
        byte_properties.remove(key);
    }

    void Document::intProperty(std::string const &key, int value) {
        int_properties.put(key, value);
    }

    int Document::intProperty(std::string const &key) const {
        return int_properties.get(key);
    }

    void Document::removeIntProperty(std::string const &key) {
        int_properties.remove(key);
    }

    void Document::longProperty(std::string const &key, long value) {
        long_properties.put(key, value);
    }

    long Document::longProperty(std::string const &key) const {
        return long_properties.get(key);
    }

    void Document::removeLongProperty(std::string const &key) {
        long_properties.remove(key);
    }

    void Document::stringProperty(std::string const &key, std::string const &value) {
        string_properties.put(key, value);
    }

    std::string Document::stringProperty(std::string const &key) const {
        return string_properties.get(key);
    }

    void Document::removeStringProperty(std::string const &key) {
        string_properties.remove(key);
    }

    yuca::utils::List<std::string> Document::propertyKeys(PropertyType type) const {
        switch (type) {
            case BOOL:
                return bool_properties.keyList();
            case BYTE:
                return byte_properties.keyList();
            case INT:
                return int_properties.keyList();
            case LONG:
                return long_properties.keyList();
            case STRING:
                return string_properties.keyList();
        }
        return utils::List<std::string>();
    }

    bool Document::operator<(const Document &other) const {
        return id < other.id;
    }

    bool Document::operator==(const Document &other) const {
        return id == other.id;
    }

    std::ostream &operator<<(std::ostream &output_stream, const Document &doc) {
        output_stream << "Document(@" << (long(&doc) % 10000) << ", id=" << doc.id << "):" << std::endl;
        // groups
        output_stream << " groups=(";
        auto groups_set = doc.getGroups();
        auto it = groups_set.begin();
        auto groups_end = groups_set.end();
        for (auto const &group : groups_set) {
            output_stream << group;
            it++;
            if (it != groups_end) {
                output_stream << ", ";
            }
        }
        output_stream << ")" << std::endl;
        output_stream.flush();

        // groups_2_keys_map
        output_stream << " group_2_keyset_map={";
        unsigned int i = 0;
        yuca::utils::Set<std::string> doc_groups = doc.group_2_keyset_map.keySet();
        for (auto const &group : doc_groups.getStdSet()) {
            SPStringKeySet keys(doc.group_2_keyset_map.get(group));
            output_stream << std::endl << "   group=<" << group << "> = ";
            output_stream << "[ ";
            unsigned long j = 0;
            for (auto const &key_sp : keys.getStdSet()) {
                output_stream << *key_sp;
                if (j < keys.size() - 1) {
                    output_stream << ", ";
                }
                j++;
            }
            output_stream << " ]";
            if (i < doc_groups.size() - 1) {
                output_stream << "," << std::endl;
            }
            i++;
        }
        output_stream << std::endl << " }";
        output_stream.flush();
        return output_stream;
    }
}
