//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include <map>
#include <set>
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
}

#endif //YUCA_DOCUMENT_HPP
