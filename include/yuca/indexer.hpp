#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "yuca/document.hpp"

#include <map>
#include <memory>

namespace yuca {

	class Indexer {
	public:
		void indexDocument(Document &doc);

	private:
		std::multimap<Key, Document*> reverse_index;
	};
}

#endif //YUCA_INDEXER_HPP
