//
// Created by gubatron on 11/9/17.
//

#include <iostream>
#include "document.hpp"

namespace yuca {
    void Document::getTags(std::set<std::string> &tagsOut) const {
        if (tagKeysMap.empty()) {
            return;
        }
        tagsOut = tags;
        /**
        auto it = tagKeysMap.begin();
        while (it != tagKeysMap.end()) {
            tagsOut.push_back(it->first);
            it++;
        }
         */
    }

    void Document::getTagKeys(std::string const &tag, KeySet &keysOut) const {
        KeySet keys;
        if (!hasKeys(tag)) {
            return;
        }
        keysOut = tagKeysMap.find(tag)->second;
    }

    void Document::addKey(const Key &key) {
        std::string tag(key.getTag());
        if (!hasKeys(tag)) {
            tagKeysMap.emplace(std::make_pair(tag, KeySet()));
        }
        tagKeysMap.find(tag)->second.insert(key);
        tags.emplace(tag);
    }

    bool Document::hasKeys(std::string const &tag) const {
        return tagKeysMap.count(tag) > 0;
    }

    void Document::removeTag(std::string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        tagKeysMap[tag].clear();
        tagKeysMap.erase(tag);
        tags.erase(tag);
    }

    void Document::removeKey(std::string const &tag, Key const &key) {
        if (!hasKeys(tag)) {
            return;
        }

        KeySet keys;
        getTagKeys(tag, keys);

        auto findIterator = keys.find(key);

        if (findIterator != keys.end()) {
            Key k = *findIterator;
            keys.erase(k);
        }

        // once we know the keySet has been cleared we remove it altogether from our { string -> [key0, key1] } map.
        if (keys.empty()) {
            removeTag(tag);
        }
    }

    bool Document::operator<(Document other) const {
        std::cout << "Document::operator< : Comparing me(" << ((long) this) << ") vs other(" << ((long) &other) << ")"
                  << std::endl;
        std::cout.flush();
        auto myMemory = (long) this;
        auto otherMemoryOffset = (long) &other;
        return myMemory < otherMemoryOffset;
    }

    bool Document::operator==(Document other) const {
        std::cout << "Document::operator== !" << std::endl;
        return (long) this == (long) &other;
    }
}