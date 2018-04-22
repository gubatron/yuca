#include "indexer.hpp"
#include <iostream>

namespace yuca {

    void ReverseIndex::putDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc) {
        if (!hasDocuments(key)) {
            index.put(key, DocumentSet());
        }
        DocumentSet docs = getDocuments(key);
        docs.add(doc);
        index.put(key,docs);
    }

    auto ReverseIndex::hasDocuments(std::shared_ptr<Key> key) const -> bool {
        return index.containsKey(key);
    }

    DocumentSet ReverseIndex::getDocuments(std::shared_ptr<Key> key) const {
        if (!hasDocuments(key)) {
            return DocumentSet();
        }
        return index.get(key);
    }

    long ReverseIndex::getKeyCount() const {
        return index.size();
    }

    std::ostream& operator<<(std::ostream &output_stream, ReverseIndex &rindex) {
        output_stream << "ReverseIndex(@" << ((long) &rindex % 10000) << "):" << std::endl;
        auto it = rindex.index.getStdMap().begin();
        if (it == rindex.index.getStdMap().end()) {
            output_stream << "<empty>" << std::endl;
        } else {
            // index = { Key => DocumentSet }
            while (it != rindex.index.getStdMap().end()) {
                output_stream << " ";
                output_stream << (*it).first;
                output_stream << " => ";
                auto docset_it = (*it).second.getStdSet().begin();
                output_stream << "(" << (*it).second.size() << ") ";
                if (docset_it == (*it).second.getStdSet().end()) {
                    output_stream << "<empty>" << std::endl;
                } else {
                    output_stream << "[";
                    while (docset_it != (*it).second.getStdSet().end()) {
                        output_stream << *docset_it;
                        docset_it++;
                        if (docset_it != (*it).second.getStdSet().end()) {
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

    DocumentSet Indexer::findDocuments(std::shared_ptr<Key> key) const {
        return getReverseIndex(key->getTag())->getDocuments(key);
    }

    DocumentSet Indexer::findDocuments(int numKeys, std::shared_ptr<Key> keys[]) const {
        DocumentSet docs_out;
        for (int i = 0; i < numKeys; i++) {
            DocumentSet docs = findDocuments(keys[i]);
            if (!docs.isEmpty()) {
                for (auto const& doc_sp : docs.getStdSet()) {
                    docs_out.add(doc_sp);
                }
            }
        }
        return docs_out;
    }

    void Indexer::addToIndex(std::string const &tag, std::shared_ptr<Document> doc) {
        KeySet doc_keys = doc->getTagKeys(tag);
        if (doc_keys.isEmpty()) {
            std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document has no doc_keys under this tag <"
                      << tag << ">" << std::endl;
            return;
        }
        // Make sure there's a ReverseIndex, if there isn't one, create an empty one
        std::shared_ptr<ReverseIndex> r_index = getReverseIndex(tag);
        if (r_index->getKeyCount() == 0) {
            reverseIndices.put(tag, std::make_shared<ReverseIndex>());
            r_index = getReverseIndex(tag);
        }
        for (auto const& k_sp : doc_keys.getStdSet()) {
            r_index->putDocument(k_sp, doc);
        }
        reverseIndices.put(tag,r_index);
    }

    std::shared_ptr<ReverseIndex> Indexer::getReverseIndex(std::string const &tag) const {
        if (!reverseIndices.containsKey(tag)) {
            return std::make_shared<ReverseIndex>();
        }
        return reverseIndices.get(tag);
    }

    std::ostream& operator<<(std::ostream &output_stream, Indexer &indexer) {
        output_stream << "Indexer(@" << ((long) &indexer % 10000) << "): " << std::endl;
        output_stream << " reverseIndices = { ";

        auto it = indexer.reverseIndices.getStdMap().begin();
        if (it == indexer.reverseIndices.getStdMap().end()) {
            output_stream << "<empty>";
        } else {
            while (it != indexer.reverseIndices.getStdMap().end()) {
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
