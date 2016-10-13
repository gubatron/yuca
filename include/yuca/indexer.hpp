#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "yuca/document.hpp"

#include <map>
#include <memory>

namespace yuca {

	class Indexer {
	public:

		void addDocument(Document doc);

	private:
		std::multimap<Document::Key, std::shared_ptr<Document>> documents;
	};
}

#endif //YUCA_INDEXER_HPP
