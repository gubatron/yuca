#include "indexer.hpp"
#include <iostream>

namespace yuca {

    void ReverseIndex::putDocument(Key const &key, Document const &doc) {
        if (!hasDocuments(key)) {
            index.emplace(std::make_pair(key, DocumentSet()));
        }
        DocumentSet docs;
        getDocuments(key, docs);
        docs.emplace(doc);
    }

    bool ReverseIndex::hasDocuments(Key const &key) const {
        return index.count(key) > 0;
    }

    void ReverseIndex::getDocuments(Key const &key, DocumentSet &docsOut) const {
        docsOut.clear();
        if (!hasDocuments(key)) {
            return;
        }
        auto it = index.find(key);
        docsOut = it->second;
    }

    long ReverseIndex::getKeyCount() const {
        return index.size();
    }


    void Indexer::indexDocument(Document const &doc) {
        std::set<std::string> tags;
        doc.getTags(tags);
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

    void Indexer::findDocuments(Key const &key, DocumentSet &docsOut) const {
        docsOut.clear();
        std::string tag = key.getTag();
        ReverseIndex rIndex;
        getReverseIndex(tag, rIndex);
        rIndex.getDocuments(key, docsOut);
    }

    void Indexer::findDocuments(int numKeys, Key keys[], DocumentSet &docsOut) const {
        for (int i = 0; i < numKeys; i++) {
            DocumentSet docs;
            findDocuments(keys[i], docs);
            if (!docs.empty()) {
                auto it = docs.begin();
                while (it != docs.end()) {
                    docsOut.insert(*it);
                    it++;
                }
            }
        }
    }

    void Indexer::addToIndex(std::string const &tag, Document const &doc) {
        KeySet keys;
        doc.getTagKeys(tag, keys);
        if (keys.empty()) {
            std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document has no keys under this tag <"
                      << tag << ">" << std::endl;
            return;
        }

        // Make sure there's a ReverseIndex, if there isn't one, create an empty one
        std::set<std::string> tags;
        doc.getTags(tags);
        ReverseIndex rIndex;
        getReverseIndex(tag, rIndex);
        if (rIndex.getKeyCount() == 0) {
            reverseIndices.emplace(std::make_pair(tag, ReverseIndex()));
            getReverseIndex(tag, rIndex);
        }

        auto keysIterator = keys.begin();
        while (keysIterator != keys.end()) {
            Key key = *keysIterator;
            rIndex.putDocument(key, doc);
            keysIterator++;
        }
    }

    void Indexer::getReverseIndex(std::string const &tag, ReverseIndex &rIndexOut) const {
        if (reverseIndices.count(tag) == 0) {
            rIndexOut = ReverseIndex();
            return;
        }
        auto rIndexIterator = reverseIndices.find(tag);
        rIndexOut = rIndexIterator->second;
    }
}
