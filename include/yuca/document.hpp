//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include "yuca/key.hpp"
#include "yuca.hpp"

namespace yuca {

	class Document {
	public:
		Document() = default;

        /** Associate this document to an indexing key under the given tag */
        void addKey(string const &tag, const Key &key);

        /** Does this document have at least one key under this tag? */
        bool hasKeys(string const &tag) const;

        /** Returns a copy of all tags under which we have KeySets */
        std::vector<string> getTags() const;

        /** Returns a copy of all keys available under a given tag */
        KeySet getTagKeys(string const &tag) const;

        /** Removes all keys under this tag */
        void removeKeys(string const &tag);

        /** Removes the given key */
        void removeKey(string const &tag, const Key &key);
    private:
        // maps tags to set<Key*>
        DocumentKeysMap documentKeys;
	};


    std::vector<string> Document::getTags() const {
        std::vector<string> ret;
        if (documentKeys.size() == 0) {
            return ret;
        }
        auto it = documentKeys.begin();
        while (it != documentKeys.end()) {
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
        auto it1 = documentKeys.find(tag);
        KeySet::iterator it = (*it1).second.begin();
        while (it != (*it1).second.end()) {
            ret.insert(*it);
            it++;
        }

        return ret;
    }

    void Document::addKey(string const &tag, const Key &key) {
        if (!hasKeys(tag)) {
            documentKeys[tag] = KeySet();
        }
        documentKeys[tag].insert((Key*) &key);
    }

    bool Document::hasKeys(string const &tag) const {
        return documentKeys.count(tag) > 0;
    }

    void Document::removeKeys(string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        documentKeys[tag].clear();
        documentKeys.erase(tag);
    }

    void Document::removeKey(string const &tag, const Key &key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet* keySet = &documentKeys[tag];
        KeySet::iterator findIterator = keySet->find((Key*)&key);

        if (findIterator != keySet->end()) {
            Key* k = *findIterator;
            keySet->erase(k);
        }

        if (keySet->size() == 0) {
            documentKeys.erase(tag);
        }
    }
}

#endif //YUCA_DOCUMENT_HPP
