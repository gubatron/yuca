//
// Created by gubatron on 11/9/17.
//

#include <iostream>
#include "document.hpp"

namespace yuca {
    std::set<std::string> Document::getTags() const {
        std::set<std::string> tags_out;
        if (tag_2_keyset_map.isEmpty()) {
            return tags_out;
        }
        return tags.getStdSet();
    }

    KeySet Document::getTagKeys(std::string const &tag) const {
        if (!hasKeys(tag)) {
            return KeySet();
        }
        return tag_2_keyset_map.get(tag);
    }

    void Document::addKey(std::shared_ptr<Key> key) {
        std::string tag(key->getTag());
        if (!hasKeys(tag)) {
            tag_2_keyset_map.put(tag, KeySet());
        }
        KeySet key_set = tag_2_keyset_map.get(tag);
        key_set.add(key);
        tag_2_keyset_map.put(tag, key_set);

        tags.add(tag);
    }

    void Document::removeKey(std::string const &tag, std::shared_ptr<Key> key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet keys = getTagKeys(tag);
        keys.remove(key);
        tag_2_keyset_map.remove(tag);
        tag_2_keyset_map.put(tag, keys);

        // once we know the keySet has been cleared we remove it altogether from our { string -> [key0, key1] } map.
        keys = getTagKeys(tag);
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
        tags.remove(tag);
    }

    bool Document::operator<(const Document &other) const {
        return creation_timestamp < other.creation_timestamp;
    }

    bool Document::operator==(const Document &other) const {
        return creation_timestamp == other.creation_timestamp;
    }

    std::ostream& operator<<(std::ostream &output_stream, const Document &doc) {
        output_stream << "Document(@" << ((long) &doc % 10000) << ", ts=" << doc.creation_timestamp << "):" << std::endl;
        // tags
        output_stream << " tags={" << doc.tags << "}" << std::endl;
        output_stream.flush();

        // tags_2_keys_map
        auto t2k_it = doc.tag_2_keyset_map.getStdMap().begin();
        output_stream << " tag_2_keyset_map={";
        while (t2k_it != doc.tag_2_keyset_map.getStdMap().end()) {
            std::string tag(t2k_it->first);
            KeySet keys(t2k_it->second);
            output_stream << std::endl << "   tag=<" << tag << "> = ";
            auto keys_it = keys.getStdSet().begin();
            while (keys_it != keys.getStdSet().end()) {
                output_stream << **keys_it;
                keys_it++;
                if (keys_it != keys.getStdSet().end()) {
                    output_stream << ", ";
                }
            }
            output_stream << " ]";
            t2k_it++;
            if (t2k_it != doc.tag_2_keyset_map.getStdMap().end()) {
                output_stream << "," << std::endl;
            }
        }
        output_stream << std::endl << " }";
        output_stream.flush();
        return output_stream;
    }
}