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

    std::set<std::string> Document::getTags() const {
        std::set<std::string> tags_out;
        if (tag_2_keyset_map.isEmpty()) {
            return tags_out;
        }
        return tag_2_keyset_map.keySet().getStdSet();
    }

    long Document::getId() const {
        return id;
    }

    StringKeySet Document::getTagKeys(std::string const &tag) const {
        StringKeySet keySet;
        SPStringKeySet spKeySet = getTagSPKeys(tag);
        for (auto const &spKey : spKeySet.getStdSet()) {
            keySet.add(*spKey);
        }
        return keySet;
    }

    SPStringKeySet Document::getTagSPKeys(std::string const &tag) const {
        if (!hasKeys(tag)) {
            return SPStringKeySet();
        }
        return tag_2_keyset_map.get(tag);
    }

    void Document::addKey(StringKey const &key) {
        auto spKey = std::make_shared<StringKey>(key);
        addKey(std::move(spKey));
    }

    void Document::addKey(SPStringKey key) {
        std::string tag(key->getTag());
        if (!hasKeys(tag)) {
            tag_2_keyset_map.put(tag, SPStringKeySet());
        }
        SPStringKeySet key_set = tag_2_keyset_map.get(tag);
        key_set.add(key);
        tag_2_keyset_map.put(tag, key_set);
    }

    void Document::removeKey(std::string const &tag, SPStringKey key) {
        if (!hasKeys(tag)) {
            return;
        }
        SPStringKeySet keys = getTagSPKeys(tag);
        keys.remove(std::move(key));
        tag_2_keyset_map.put(tag, keys);

        // once we know the keySet has been cleared we remove it altogether from our { string -> [key0, key1] } map.
        keys = getTagSPKeys(tag);
        if (keys.isEmpty()) {
            removeTag(tag);
        }
    }

    bool Document::hasKeys(std::string const &tag) const {
        return tag_2_keyset_map.containsKey(tag);
    }

    void Document::removeTag(std::string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        tag_2_keyset_map.get(tag).clear();
        tag_2_keyset_map.remove(tag);
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
        // tags
        output_stream << " tags=(";
        auto tags_set = doc.getTags();
        auto it = tags_set.begin();
        auto tags_end = tags_set.end();
        for (auto const &tag : tags_set) {
            output_stream << tag;
            it++;
            if (it != tags_end) {
                output_stream << ", ";
            }
        }
        output_stream << ")" << std::endl;
        output_stream.flush();

        // tags_2_keys_map
        output_stream << " tag_2_keyset_map={";
        unsigned int i = 0;
        yuca::utils::Set<std::string> doc_tags = doc.tag_2_keyset_map.keySet();
        for (auto const &tag : doc_tags.getStdSet()) {
            SPStringKeySet keys(doc.tag_2_keyset_map.get(tag));
            output_stream << std::endl << "   tag=<" << tag << "> = ";
            output_stream << "[ ";
            unsigned long j = 0;
            for (auto const &key_sp : keys.getStdSet()) {
                // bad design hack, this should happen automatically, it should
                // use the << operator of the child Key class
                auto cast_key = std::dynamic_pointer_cast<StringKey>(key_sp);
                if (cast_key != nullptr) {
                    output_stream << *cast_key;
                } else {
                    output_stream << *key_sp;
                }
                if (j < keys.size() - 1) {
                    output_stream << ", ";
                }
                j++;
            }
            output_stream << " ]";
            if (i < doc_tags.size() - 1) {
                output_stream << "," << std::endl;
            }
            i++;
        }
        output_stream << std::endl << " }";
        output_stream.flush();
        return output_stream;
    }
}
