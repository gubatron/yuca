//
// Created by Alden Torres on 10/13/16.
//

#ifndef YUCA_DOCUMENT_HPP
#define YUCA_DOCUMENT_HPP

#include <vector>

namespace yuca {

	class Document {
	public:

		class Key {

		};

		Document(int) {

		}

		Document() = default;

		std::vector<Key> generateKeys();
	};
}

namespace std
{
	template <>
	struct hash<yuca::Document::Key>
	{
		std::size_t operator()(libtorrent::sha1_hash const& k) const
		{
			std::size_t ret;
			// this is OK because sha1_hash is already a hash
			std::memcpy(&ret, &k[0], sizeof(ret));
			return ret;
		}
	};
}

#endif //YUCA_DOCUMENT_HPP
