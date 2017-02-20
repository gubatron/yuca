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
		 * The Indexer is conformed by multiple reverse indexes,
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

	// IMPLEMENTATION

	void Indexer::indexDocument(const Document &doc) {

		DocumentKeys keys = doc.getKeys();
		DocumentKeys::iterator it = keys.begin();

		// look for each one of the keys defined for this document
		// the keys come along with their tag, which is used
		// by the indexer to partition the reverse indexes
		//
		// Document -> { tagName -> [ Key, Key2, ..., KeyN ], ... }
		//
		//
		// REVERSE_ENGINES[tag] ->
		//      \_
		//        [key1] = [Document1, Document2, ... ]
		//        [key2] = [Document1, Document2, ... ]
		while (it != keys.end()) {
			std::string tag = it->first;
			std::cout << "Indexer::indexDocument: tag [" << tag << "]";
			ReverseIndex rIndex = getReverseIndex(tag);
			addToIndex(tag, doc);
			it++;
		}
	}

	void Indexer::addToIndex(std::string const& tag, const Document &doc) {
		DocumentKeys keys = doc.getKeys();
		DocumentKeys::iterator keysIterator = keys.find(tag);
		if (keysIterator == keys.end()) {
			std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document does not use this tag" << std::endl;
			return;
		}
		KeySet docKeys = keysIterator->second;
		ReverseIndex rIndex = getReverseIndex(tag);
		rIndex.addAll(docKeys, (Document *) &doc); // not sure if this will fly
	}

	ReverseIndex Indexer::getReverseIndex(std::string const& tag) {
		if (reverseIndices.count(tag) == 0) {
			// return a new map
			reverseIndices[tag] = ReverseIndex();
			return reverseIndices[tag];
		}
		std::map<std::string, ReverseIndex>::iterator
				rIndexIterator = reverseIndices.find(tag);
		return rIndexIterator->second;
	}
}

#endif //YUCA_INDEXER_HPP
