#include <iostream>
#include "yuca/indexer.hpp"

namespace yuca {
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
