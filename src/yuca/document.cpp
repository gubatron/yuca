//
// Created by gubatron on 11/9/17.
//

#include "document.hpp"

namespace yuca {
    std::vector<std::string> Document::getTags() const {
        std::vector<std::string> ret;
        if (tagKeysMap.empty()) {
            return ret;
        }
        auto it = tagKeysMap.begin();
        while (it != tagKeysMap.end()) {
            ret.push_back((*it).first);
            it++;
        }
        return ret;
    }

    KeySet Document::getTagKeys(std::string const &tag) const {
        KeySet ret;
        if (!hasKeys(tag)) {
            return ret;
        }
        auto tagKeysMapIterator = tagKeysMap.find(tag);
        auto keysetIterator = (*tagKeysMapIterator).second.begin();
        auto lastKey = (*tagKeysMapIterator).second.end();
        while (keysetIterator != lastKey) {
            ret.insert(*keysetIterator);
            keysetIterator++;
        }

        return ret;
    }

    void Document::addKey(const Key &key) {
        std::string tag(key.getTag());

        if (!hasKeys(tag)) {
            tagKeysMap[tag] = KeySet();
        }
        tagKeysMap[tag].insert((Key *) &key);
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
    }

    void Document::removeKey(std::string const &tag, const Key &key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet *keySet = &tagKeysMap[tag];
        auto findIterator = keySet->find((Key *) &key);

        if (findIterator != keySet->end()) {
            Key *k = *findIterator;
            keySet->erase(k);
        }

        if (!keySet->empty()) {
            tagKeysMap.erase(tag);
        }
    }
}