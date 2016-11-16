#include "yuca/indexer.hpp"

namespace yuca {

	void Indexer::indexDocument(Document &doc) {
        std::vector<Key> keys = doc.generateKeys();
        for (Key key : keys) {
            reverse_index.emplace(key, &doc);
            reverse_index.insert(std::pair<Key, Document*>(key, &doc));
        }
	}
}
