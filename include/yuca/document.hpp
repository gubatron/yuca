//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include <vector>
#include "key.hpp"

namespace yuca {
    typedef std::string string;
    typedef std::set<Key *> KeySet;

    class Document {
	public:
		Document() = default;

        /** Associate this document to an indexing key under the given tag */
        void addKey(const Key &key);

        /** Does this document have at least one key under this tag? */
        bool hasKeys(string const &tag) const;

        /** Returns a copy of all tags under which we have KeySets */
        std::vector<string> getTags() const;

        /** Returns a copy of all keys available under a given tag */
        KeySet getTagKeys(string const &tag) const;

        /** Removes all keys under this tag */
        void removeTag(string const &tag);

        /** Removes the given key. If it's the last key, the tag is removed */
        void removeKey(string const &tag, const Key &key);
    private:
        // maps tags to set<Key*>
        std::map<string, KeySet> tagKeysMap;
	};


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
        tagKeysMap[tag].insert((Key*) &key);
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
        KeySet* keySet = &tagKeysMap[tag];
        KeySet::iterator findIterator = keySet->find((Key*)&key);

        if (findIterator != keySet->end()) {
            Key* k = *findIterator;
            keySet->erase(k);
        }

        if (keySet->size() == 0) {
            tagKeysMap.erase(tag);
        }
    }
}

#endif //YUCA_DOCUMENT_HPP
