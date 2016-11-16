//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include "yuca/key.h"

#include <libtorrent/sha1_hash.hpp>
#include <vector>

namespace yuca {

	class Document {
	public:
		Document(int) {
		}

		Document() = default;

		virtual std::vector<Key> generateKeys() const;

	};
}

#endif //YUCA_DOCUMENT_HPP
