#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "yuca/document.hpp"

#include <map>
#include <memory>

namespace yuca {

	struct ReverseIndex {
		std::map<Key *, std::set<Document *>> index;

		void put(Key *key, Document *doc) {
			std::set<Document *> documents = get(key);
			documents.insert(doc);
		}

		void addAll(KeySet keySet, Document *doc) {
			KeySet::iterator keysIterator = keySet.begin();

		    while (keysIterator != keySet.end()) {
				Key* key = *keysIterator;
				get(key);

				keysIterator++;
			}
		}

		bool has(Key *key) {
			return index.count(key) > 0;
		}

		DocumentSet get(Key *key) {
		    if (!has(key))	{
				index[key] = std::set<Document *>();
				return index[key];
			}
			std::map<Key *, std::set<Document *>>::iterator it = index.find(key);
			// *it dereferences to a pair<Key*, std::vector<Document *>>
			return (*it).second;
		}
	};

	class Indexer {
	public:
		void indexDocument(const Document &doc);

	private:
		std::map<std::string, ReverseIndex> reverseIndices;

		/**
		 * The Indexer is confirmed by multiple reverse indexes,
		 * which are identified by a 'tag', this helps us partition our indexing
		 * by categories.
		 *
		 * For example, we could have a reverse index that catalogs documents by "file_extension"
		 *
		 * Inside the "file_extension" reverse index, we will find a multimap
		 * of Key -> [Document, ...]
		 *
		 * Documents provide the indexer with the Keys to be used.
		 *
		 * */
		ReverseIndex getReverseIndex(std::string const& tag);

		void addToIndex(std::string const& tag, const Document &doc);
	};
}

#endif //YUCA_INDEXER_HPP
