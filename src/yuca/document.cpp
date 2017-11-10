//
// Created by gubatron on 11/9/17.
//

#include "document.hpp"

namespace yuca {
    std::vector<string> Document::getTags() const {
        std::vector<string> ret;
        if (tagKeysMap.size() == 0) {
            return ret;
        }
        auto it = tagKeysMap.begin();
        while (it != tagKeysMap.end()) {
            ret.emplace_back((*it).first);
            it++;
        }
        return ret;
    }

    KeySet Document::getTagKeys(string const &tag) const {
        KeySet ret;
        if (!hasKeys(tag)) {
            return ret;
        }
        auto it1 = tagKeysMap.find(tag);
        KeySet::iterator it = (*it1).second.begin();
        while (it != (*it1).second.end()) {
            ret.insert(*it);
            it++;
        }

        return ret;
    }

    void Document::addKey(const Key &key) {
        std::string tag(key.getTag());

        if (!hasKeys(tag)) {
            tagKeysMap[tag] = KeySet();
        }
        tagKeysMap[tag].emplace((Key *) &key);
    }

    bool Document::hasKeys(string const &tag) const {
        return tagKeysMap.count(tag) > 0;
    }

    void Document::removeTag(string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        tagKeysMap[tag].clear();
        tagKeysMap.erase(tag);
    }

    void Document::removeKey(string const &tag, const Key &key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet *keySet = &tagKeysMap[tag];
        KeySet::iterator findIterator = keySet->find((Key *) &key);

        if (findIterator != keySet->end()) {
            Key *k = *findIterator;
            keySet->erase(k);
        }

        if (keySet->size() == 0) {
            tagKeysMap.erase(tag);
        }
    }
}