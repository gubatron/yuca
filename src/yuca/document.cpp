//
// Created by gubatron on 11/9/17.
//

#include <iostream>
#include "document.hpp"

namespace yuca {
    std::set<std::string> Document::getTags() const {
        std::set<std::string> tags_out;
        if (tag_2_keyset_map.empty()) {
            return tags_out;
        }
        return tags.getStdSet();
    }

    KeySet Document::getTagKeys(std::string const &tag) const {
        if (!hasKeys(tag)) {
            return KeySet();
        }
        return tag_2_keyset_map.find(tag)->second;
    }

    void Document::addKey(std::shared_ptr<Key> key) {
        std::string tag(key->getTag());
        if (!hasKeys(tag)) {
            tag_2_keyset_map.emplace(std::make_pair(tag, KeySet()));
        }
        auto it = tag_2_keyset_map.find(tag);
        if (it != tag_2_keyset_map.end()) {
            tag_2_keyset_map[tag].add(key);
        }
        tags.add(tag);
    }

    void Document::removeKey(std::string const &tag, std::shared_ptr<Key> key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet keys = getTagKeys(tag);
        keys.remove(key);
        tag_2_keyset_map.erase(tag);
        tag_2_keyset_map.emplace(std::make_pair(tag, keys));

        // once we know the keySet has been cleared we remove it altogether from our { string -> [key0, key1] } map.
        keys = getTagKeys(tag);
        if (keys.isEmpty()) {
            removeTag(tag);
        }
    }


    bool Document::hasKeys(std::string const &tag) const {
        return tag_2_keyset_map.count(tag) > 0;
    }

    void Document::removeTag(std::string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        tag_2_keyset_map[tag].clear();
        tag_2_keyset_map.erase(tag);
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
        auto t2k_it = doc.tag_2_keyset_map.begin();
        output_stream << " tag_2_keyset_map={";
        while (t2k_it != doc.tag_2_keyset_map.end()) {
            std::string tag(t2k_it->first);
            KeySet keys(t2k_it->second);
            output_stream << std::endl << "   tag=<" << tag << "> = ";
            output_stream << keys;
//            auto keys_it = keys.begin();
//            while (keys_it != keys.end()) {
//                output_stream << *(*keys_it).get();
//                keys_it++;
//                if (keys_it != keys.end()) {
//                    output_stream << ", ";
//                }
//            }
//            output_stream << " ]";
            t2k_it++;
            if (t2k_it != doc.tag_2_keyset_map.end()) {
                output_stream << "," << std::endl;
            }
        }
        output_stream << std::endl << " }";
        output_stream.flush();
        return output_stream;
    }
}