#include "indexer.hpp"
#include <iostream>

namespace yuca {

    void ReverseIndex::putDocument(Key *key, Document *doc) {
        if (!hasDocuments(key)) {
            index[key] = std::set<Document *>();
        }
        index[key].insert(doc);
    }

    bool ReverseIndex::hasDocuments(Key *key) const {
        return index.count(key) > 0;
    }

    DocumentSet ReverseIndex::getDocuments(Key *key) const {
        if (!hasDocuments(key)) {
            return DocumentSet();
        }
        auto it = index.find(key);
        return (*it).second;
    }

    long ReverseIndex::getKeyCount() const {
        return index.size();
    }


    void Indexer::indexDocument(Document const &doc) {
        std::vector<std::string> tags = doc.getTags();
        auto tags_iterator = tags.begin();
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
        // REVERSE_INDICES[tag] ->
        //      \_
        //        [key1] = [Document1, Document2, ... ]
        //        [key2] = [Document1, Document2, ... ]
    }

    void Indexer::removeDocument(Document const &doc) {
        // TODO
    }

    DocumentSet Indexer::findDocuments(Key &key) const {
        std::string tag = key.getTag();
        ReverseIndex *const rIndex = getReverseIndex(tag);
        return rIndex->getDocuments(&key);
    }

    DocumentSet Indexer::findDocuments(int numKeys, Key keys[]) const {
        DocumentSet result;
        for (int i = 0; i < numKeys; i++) {
            DocumentSet docs = findDocuments(keys[i]);
            if (!docs.empty()) {
                auto it = docs.begin();
                while (it != docs.end()) {
                    result.insert(*it);
                    it++;
                }
            }
        }
        return result;
    }

    void Indexer::addToIndex(std::string const &tag, const Document &doc) {
        KeySet keys = doc.getTagKeys(tag);
        auto keysIterator = keys.begin();
        if (keysIterator == keys.end()) {
            std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document has no keys under this tag <"
                      << tag << ">" << std::endl;
            return;
        }
        ReverseIndex *rIndex = getReverseIndex(tag);
        if (rIndex->getKeyCount() == 0) {
            reverseIndices[tag] = rIndex;
        }
        while (keysIterator != keys.end()) {
            Key *key = *keysIterator;
            rIndex->putDocument(key, (Document *) &doc);
            keysIterator++;
        }
    }

    ReverseIndex *const Indexer::getReverseIndex(std::string const &tag) const {
        if (reverseIndices.count(tag) == 0) {
            return new ReverseIndex();
        }
        auto rIndexIterator = reverseIndices.find(tag);
        return rIndexIterator->second;
    }
}
