#include "indexer.hpp"
#include <iostream>

namespace yuca {

    void ReverseIndex::putDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc) {
        if (!hasDocuments(key)) {
            index.emplace(std::make_pair(key, DocumentSet()));
        }
        DocumentSet docs;
        getDocuments(key, docs);
        docs.emplace(doc);
        index[key]=docs;
    }

    auto ReverseIndex::hasDocuments(std::shared_ptr<Key> key) const -> bool {
        return index.count(key) > 0;
    }

    void ReverseIndex::getDocuments(std::shared_ptr<Key> key, DocumentSet &docs_out) const {
        docs_out.clear();
        if (!hasDocuments(key)) {
            return;
        }
        auto it = index.find(key);
        docs_out = it->second;
    }

    long ReverseIndex::getKeyCount() const {
        return index.size();
    }

    std::ostream& operator<<(std::ostream &output_stream, ReverseIndex &rindex) {
        output_stream << "ReverseIndex(@" << ((long) &rindex % 10000) << "):" << std::endl;
        auto it = rindex.index.begin();
        if (it == rindex.index.end()) {
            output_stream << "<empty>" << std::endl;
        } else {
            // index = { Key => DocumentSet }
            while (it != rindex.index.end()) {
                output_stream << " ";
                output_stream << (*it).first;
                output_stream << " => ";
                auto docset_it = (*it).second.begin();
                output_stream << "(" << (*it).second.size() << ") ";
                if (docset_it == (*it).second.end()) {
                    output_stream << "<empty>" << std::endl;
                } else {
                    output_stream << "[";
                    while (docset_it != (*it).second.end()) {
                        output_stream << *docset_it;
                        docset_it++;
                        if (docset_it != (*it).second.end()) {
                            output_stream << ", ";
                        }
                    }
                    output_stream << "]" << std::endl;
                }
                it++;
            }
        }
        output_stream.flush();
        return output_stream;
    }


    void Indexer::indexDocument(std::shared_ptr<Document> doc) {
        std::set<std::string> tags = doc->getTags();
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

    void Indexer::removeDocument(std::shared_ptr<Document> doc) {

    }

    void Indexer::findDocuments(std::shared_ptr<Key> key, DocumentSet &docs_out) const {
        docs_out.clear();
        std::string tag = key->getTag();
        std::shared_ptr<ReverseIndex> rIndex;
        getReverseIndex(tag, rIndex);
        rIndex->getDocuments(key, docs_out);
    }

    void Indexer::findDocuments(int numKeys, std::shared_ptr<Key> keys[], DocumentSet &docs_out) const {
        for (int i = 0; i < numKeys; i++) {
            DocumentSet docs;
            findDocuments(keys[i], docs);
            if (!docs.empty()) {
                auto it = docs.begin();
                while (it != docs.end()) {
                    docs_out.insert(*it);
                    it++;
                }
            }
        }
    }

    void Indexer::addToIndex(std::string const &tag, std::shared_ptr<Document> doc) {
        KeySet doc_keys;
        doc->getTagKeys(tag, doc_keys);
        if (doc_keys.empty()) {
            std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document has no doc_keys under this tag <"
                      << tag << ">" << std::endl;
            return;
        }

        // Make sure there's a ReverseIndex, if there isn't one, create an empty one
        std::shared_ptr<ReverseIndex> r_index;
        getReverseIndex(tag, r_index);
        //std::cout << r_index;
        if (r_index->getKeyCount() == 0) {
            reverseIndices.emplace(std::make_pair(tag, std::make_shared<ReverseIndex>()));
            getReverseIndex(tag, r_index);
        }
        //std::cout << r_index;

        auto doc_keys_iterator = doc_keys.begin();
        while (doc_keys_iterator != doc_keys.end()) {
            std::shared_ptr<Key> k_sp = *doc_keys_iterator;
            r_index->putDocument(k_sp, doc);
            doc_keys_iterator++;
        }
        reverseIndices[tag] = r_index;
    }

    void Indexer::getReverseIndex(std::string const &tag, std::shared_ptr<ReverseIndex> &r_index_out) const {
        if (reverseIndices.count(tag) == 0) {
            r_index_out = std::make_shared<ReverseIndex>();
            return;
        }
        auto rIndexIterator = reverseIndices.find(tag);
        r_index_out = rIndexIterator->second;
    }

    std::ostream& operator<<(std::ostream &output_stream, Indexer &indexer) {
        output_stream << "Indexer(@" << ((long) &indexer % 10000) << "): " << std::endl;
        output_stream << " reverseIndices = { ";

        auto it = indexer.reverseIndices.begin();
        if (it == indexer.reverseIndices.end()) {
            output_stream << "<empty>";
        } else {
            while (it != indexer.reverseIndices.end()) {
                output_stream << "   " << (*it).first << " => ";
                output_stream << (*it).second; // second is the ReverseIndex
                output_stream << std::endl;
                it++;
            }
        }
        output_stream << "}";
        output_stream.flush();
        return output_stream;
    }
}
