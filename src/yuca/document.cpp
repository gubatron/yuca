//
// Created by gubatron on 11/9/17.
//

#include <iostream>
#include "document.hpp"

namespace yuca {
    void Document::getTags(std::set<std::string> &tags_out) const {
        tags_out.clear();
        if (tag_2_keyset_map.empty()) {
            return;
        }
        tags_out = tags;
    }

    void Document::getTagKeys(std::string const &tag, KeySet &keys_out) const {
        keys_out.clear();
        KeySet keys;
        if (!hasKeys(tag)) {
            return;
        }
        keys_out = tag_2_keyset_map.find(tag)->second;
    }

    void Document::addKey(std::shared_ptr<Key> key) {
        std::string tag(key->getTag());
        if (!hasKeys(tag)) {
            tag_2_keyset_map.emplace(std::make_pair(tag, KeySet()));
        }
        auto it = tag_2_keyset_map.find(tag);
        if (it != tag_2_keyset_map.end()) {
            tag_2_keyset_map[tag].insert(key);
        }
        tags.emplace(tag);
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
        tags.erase(tag);
    }

    void Document::removeKey(std::string const &tag, std::shared_ptr<Key> key) {
        if (!hasKeys(tag)) {
            return;
        }

        KeySet keys;
        getTagKeys(tag, keys);

        auto findIterator = keys.find(key);

        if (findIterator != keys.end()) {
            tag_2_keyset_map[tag].erase(key);
        }

        // once we know the keySet has been cleared we remove it altogether from our { string -> [key0, key1] } map.
        getTagKeys(tag, keys);
        if (keys.empty()) {
            removeTag(tag);
        }
    }

    bool Document::operator<(const Document &other) const {
        //TODO: revise this
        auto myMemory = (long) this;
        auto otherMemoryOffset = (long) &other;
        return myMemory < otherMemoryOffset;
    }

    bool Document::operator==(const Document &other) const {
        //TODO: revise this
        return (long) this == (long) &other;
    }

    void Document::dumpToStream(std::ostream &output_stream) const {
        output_stream << "Document(@" << ((long) this % 10000) << "):" << std::endl;
        // tags
        output_stream << " tags={ ";
        auto tags_it = tags.begin();
        while (tags_it != tags.end()) {
            output_stream << *tags_it;
            tags_it++;
            if (tags_it != tags.end()) {
                output_stream << ", ";
            }
        }
        output_stream << " }" << std::endl;
        output_stream.flush();

        // tags_2_keys_map
        auto t2k_it = tag_2_keyset_map.begin();
        output_stream << " tag_2_keyset_map={";
        while (t2k_it != tag_2_keyset_map.end()) {
            std::string tag(t2k_it->first);
            KeySet keys(t2k_it->second);
            output_stream << std::endl << "   tag=<" << tag << "> = [ ";
            auto keys_it = keys.begin();
            while (keys_it != keys.end()) {
                (*keys_it)->dumpToStream(output_stream);
                keys_it++;
                if (keys_it != keys.end()) {
                    output_stream << ", ";
                }
            }
            output_stream << " ]";
            t2k_it++;
            if (t2k_it != tag_2_keyset_map.end()) {
                output_stream << "," << std::endl;
            } /*else {
                output_stream << std::endl;
            }*/
        }
        output_stream << std::endl << " }";
        output_stream.flush();
    }
}