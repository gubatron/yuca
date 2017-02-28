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
            if (!hasDocuments(key))	{
                index[key] = std::set<Document *>();
            }
            index[key].insert(doc);
		}

		bool hasDocuments(Key *key) const {
			return index.count(key) > 0;
		}

		DocumentSet getDocuments(Key *key) const {
		    if (!hasDocuments(key))	{
				return DocumentSet();
			}
            std::map<Key *, DocumentSet>::const_iterator it = index.find(key);
            return (*it).second;
		}

        long getKeyCount() const {
            return index.size();
        }
	};

	class Indexer {
        public:
            void indexDocument(Document const &doc);
            DocumentSet findDocuments(Key &key) const;

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
            ReverseIndex* const getReverseIndex(std::string const &tag) const;

            void addToIndex(std::string const &tag, const Document &doc);

            std::map<std::string, ReverseIndex *> reverseIndices;
	};

	// IMPLEMENTATION

	void Indexer::indexDocument(Document const &doc) {

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

    DocumentSet Indexer::findDocuments(Key &key) const {
        std::string tag = key.getTag();
        ReverseIndex* const rIndex = getReverseIndex(tag);
        return rIndex->getDocuments((Key*) &key);
    }

	void Indexer::addToIndex(std::string const &tag, const Document &doc) {
		KeySet keys = doc.getTagKeys(tag);
        KeySet::iterator keysIterator = keys.begin();
		if (keysIterator == keys.end()) {
			std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document has no keys under this tag <" << tag << ">" << std::endl;
			return;
		}
        ReverseIndex *rIndex = getReverseIndex(tag);
        if (rIndex->getKeyCount() == 0) {
            reverseIndices[tag] = rIndex;
        }
        while (keysIterator != keys.end()) {
            Key* key = *keysIterator;
            rIndex->putDocument(key, (Document*) &doc);
            keysIterator++;
        }
	}

	ReverseIndex* const Indexer::getReverseIndex(std::string const& tag) const {
		if (reverseIndices.count(tag) == 0) {
			return new ReverseIndex();
		}
		std::map<std::string, ReverseIndex*>::const_iterator rIndexIterator = reverseIndices.find(tag);
		return rIndexIterator->second;
	}
}

#endif //YUCA_INDEXER_HPP
