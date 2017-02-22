#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include <map>
#include <memory>
#include <set>

namespace yuca {

    typedef std::set<Document *> DocumentSet;

	struct ReverseIndex {
		std::map<Key *, DocumentSet> index;

		void putDocument(Key *key, Document *doc) {
			DocumentSet *documents = getDocuments(key);
			documents->insert(doc);
		}

		bool hasDocuments(Key *key) const {
			return index.count(key) > 0;
		}

		DocumentSet* const getDocuments(Key *key) const {
		    if (!hasDocuments(key))	{
				index[key] = std::set<Document *>();
				return &index[key];
			}
			std::map<Key *, DocumentSet>::const_iterator it = index.find(key);
			// *it dereferences to a pair<Key*, std::vector<Document *>>
			return &(*it).second;
		}
	};

	class Indexer {
        public:
            void indexDocument(const Document &doc);

        private:
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
            ReverseIndex* const getReverseIndex(std::string const &tag);

            void addToIndex(std::string const &tag, const Document &doc);

            std::map<std::string, ReverseIndex *> reverseIndices;
	};

	// IMPLEMENTATION

	void Indexer::indexDocument(const Document &doc) {

		std::vector<string> tags = doc.getTags();
		std::vector<string>::iterator tags_iterator = tags.begin();
        while (tags_iterator != tags.end()) {
            std::string tag = *tags_iterator;
            addToIndex(tag, doc);
            tags_iterator++;
        }

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
	}

	void Indexer::addToIndex(std::string const& tag, const Document &doc) {
		KeySet keys = doc.getTagKeys(tag);
        KeySet::iterator keysIterator = keys.begin();
		if (keysIterator == keys.end()) {
			std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document does not use this tag" << std::endl;
			return;
		}
        while (keysIterator != keys.end()) {
            ReverseIndex *rIndex = getReverseIndex(tag);
            Key* key = *keysIterator;
            rIndex->putDocument(key, (Document*) &doc);
            keysIterator++;
        }
	}

	ReverseIndex* const Indexer::getReverseIndex(std::string const& tag) {
		if (reverseIndices.count(tag) == 0) {
			reverseIndices[tag] = new ReverseIndex();
			return reverseIndices[tag];
		}
		std::map<std::string, ReverseIndex*>::iterator rIndexIterator = reverseIndices.find(tag);
		return rIndexIterator->second;
	}
}

#endif //YUCA_INDEXER_HPP
