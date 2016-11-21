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

		/** Returns all keys available */
        DocumentKeys getKeys() const;

        /** Returns all keys available under a given tag */
        KeySet getKeys(string const &tag);

        /** Associate this document to an indexing key under the given tag */
        void addKey(string const &tag, const Key &key);

        /** Does this document have at least one key under this tag? */
        bool hasKeys(string const &tag) const;

        /** Removes all keys under this tag */
        void removeKeys(string const &tag);

        /** Removes the given key */
        void removeKey(string const &tag, const Key &key);
    private:
        // maps tags to set<Key*>
        DocumentKeys documentKeys;
	};

    DocumentKeys Document::getKeys() const {
        return documentKeys;
    }

    KeySet Document::getKeys(string const &tag) {
        if (!hasKeys(tag)) {
            documentKeys[tag] = KeySet();
            return documentKeys[tag];
        }
        return KeySet();
    }

    void Document::addKey(string const &tag, const Key &key) {
        KeySet keySet = getKeys(tag);
        keySet.insert((Key*) &key);
    }

    bool Document::hasKeys(string const &tag) const {
        return documentKeys.count(tag) > 0;
    }

    void Document::removeKeys(string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet keySet = getKeys(tag);
        keySet.clear();
    }

    void Document::removeKey(string const &tag, const Key &key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet keySet = getKeys(tag);
        KeySet::iterator findIterator = keySet.find((Key*)&key);

        if (findIterator != keySet.end()) {
            Key* k = *findIterator;
            keySet.erase(k);
        }
    }
}

#endif //YUCA_DOCUMENT_HPP
