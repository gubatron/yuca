#include "yuca/indexer.hpp"

namespace yuca {

	void Indexer::addDocument(Document doc) {

		auto ptr = std::make_shared<Document>(std::move(doc));
		for (auto const& k : doc.generateKeys()) {
			documents.emplace(k, ptr);
		}
		//Documentdocuments.push_back(std::move(doc));
	}
}
