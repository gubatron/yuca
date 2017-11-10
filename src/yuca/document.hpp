//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include <map>
#include <set>
#include <vector>
#include "key.hpp"
#include "types.hpp"

namespace yuca {

    class Document {
	public:
		Document() = default;

        /** Associate this document to an indexing key under the given tag */
        void addKey(const Key &key);

        /** Does this document have at least one key under this tag? */
        bool hasKeys(std::string const &tag) const;

        /** Returns a copy of all tags under which we have KeySets */
        std::vector<std::string> getTags() const;

        /** Returns a copy of all keys available under a given tag */
        KeySet getTagKeys(std::string const &tag) const;

        /** Removes all keys under this tag */
        void removeTag(std::string const &tag);

        /** Removes the given key. If it's the last key, the tag is removed */
        void removeKey(std::string const &tag, const Key &key);
    private:
        // maps tags to set<Key*>
        std::map<std::string, KeySet> tagKeysMap;
	};
}

#endif //YUCA_DOCUMENT_HPP
