//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include "yuca/key.h"
#include "yuca.h"


#include <libtorrent/sha1_hash.hpp>

namespace yuca {

	class Document {
	public:
		Document() = default;

		/** Returns all keys available */
        DocumentKeys getKeys() const;

        /** Returns all keys available under a given tag */
        KeySet getKeys(string const &name) const;

        /** Associate this document to an indexing key under the given tag */
        void addKey(string const &tag, const Key &key);

        /** Does this document have at least one key under this tag? */
        bool hasKey(string const &tag) const;

        /** Removes all keys under this tag */
        void removeKeys(string const &tag);

        /** Removes the given key */
        void removeKey(string const &tag, const Key &key);
    private:
        DocumentKeys documentKeys;
	};
}

#endif //YUCA_DOCUMENT_HPP
