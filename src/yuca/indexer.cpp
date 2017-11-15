#include "indexer.hpp"
#include <iostream>

namespace yuca {

    void ReverseIndex::putDocument(Key const &key, Document const &doc) {
        if (!hasDocuments(key)) {
            index.emplace(std::make_pair(key, DocumentSet()));
        }
        DocumentSet docs;
        getDocuments(key, docs);
        auto doc_dp = std::make_shared<Document>(doc);
        docs.emplace(doc_dp);
        index[key]=docs;
    }

    bool ReverseIndex::hasDocuments(Key const &key) const {
        return index.count(key) > 0;
    }

    void ReverseIndex::getDocuments(Key const &key, DocumentSet &docs_out) const {
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

    void ReverseIndex::dumpToStream(std::ostream &output_stream) const {
        output_stream << "ReverseIndex(@" << (long) this << "):" << std::endl;
        auto it = index.begin();
        if (it == index.end()) {
            output_stream << "<empty>" << std::endl;
        } else {
            // index = { Key => DocumentSet }
            while (it != index.end()) {
                output_stream << " ";
                (*it).first.dumpToStream(output_stream);
                output_stream << " => ";
                auto docset_it = (*it).second.begin();
                if (docset_it == (*it).second.end()) {
                    output_stream << "<empty>" << std::endl;
                } else {
                    output_stream << "[";
                    while (docset_it != (*it).second.end()) {
                        (*docset_it)->dumpToStream(output_stream);
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

    void Indexer::findDocuments(Key const &key, DocumentSet &docs_out) const {
        docs_out.clear();
        std::string tag = key.getTag();
        ReverseIndex rIndex;
        getReverseIndex(tag, rIndex);
        rIndex.getDocuments(key, docs_out);
    }

    void Indexer::findDocuments(int numKeys, Key keys[], DocumentSet &docs_out) const {
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

    void Indexer::addToIndex(std::string const &tag, Document const &doc) {
        KeySet doc_keys;
        doc.getTagKeys(tag, doc_keys);
        if (doc_keys.empty()) {
            std::cout << "Indexer::addToIndex(" << tag << "): check your logic, document has no doc_keys under this tag <"
                      << tag << ">" << std::endl;
            return;
        }

        std::cout << "Indexer::addToIndex(tag=" << tag << " doc=";
        doc.dumpToStream(std::cout);
        std::cout << ")" << std::endl;
        std::cout.flush();
        std::cout << "How does the index look before adding the document?" << std::endl;
        std::cout.flush();
        dumpToStream(std::cout);
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout.flush();

        // Make sure there's a ReverseIndex, if there isn't one, create an empty one
        ReverseIndex r_index;
        getReverseIndex(tag, r_index);
        if (r_index.getKeyCount() == 0) {
            reverseIndices.emplace(std::make_pair(tag, std::make_shared<ReverseIndex>(ReverseIndex())));
            getReverseIndex(tag, r_index);
            std::cout << "We've created a fresh new ReverseIndex on tag=" << tag << ", how does it look now?" << std::endl;
            std::cout.flush();
            r_index.dumpToStream(std::cout);
        }

        getReverseIndex(tag, r_index); // TODO: Check if we don't need this line anymore
        auto doc_keys_iterator = doc_keys.begin();
        while (doc_keys_iterator != doc_keys.end()) {
            std::shared_ptr<Key> k_sp = *doc_keys_iterator;
            r_index.putDocument(*k_sp, doc);
            std::cout << "    r_index.putDocument(key=";
            k_sp->dumpToStream(std::cout);
            std::cout << ", doc=";
            doc.dumpToStream(std::cout);
            std::cout << ")" << std::endl;
            std::cout.flush();
            std::cout << std::endl;
            std::cout << "r_index after putDocument -> " << std::endl;
            r_index.dumpToStream(std::cout);
            std::cout << "------" << std::endl;
            doc_keys_iterator++;
        }
        reverseIndices[tag] = std::make_shared<ReverseIndex>(r_index);

        std::cout << std::endl << " documents added to r_index" << std::endl;
        std::cout << "How does the index look now?" << std::endl;
        std::cout.flush();
        dumpToStream(std::cout);
        std::cout << std::endl;
        std::cout.flush();
    }

    void Indexer::getReverseIndex(std::string const &tag, ReverseIndex &r_index_out) const {
        if (reverseIndices.count(tag) == 0) {
            r_index_out = ReverseIndex();
            return;
        }
        auto rIndexIterator = reverseIndices.find(tag);
        r_index_out = *rIndexIterator->second;
    }

    void Indexer::dumpToStream(std::ostream &output_stream) const {
        //std::map<std::string, ReverseIndex> reverseIndices;
        output_stream << "Indexer(@" << (long) this << "): " << std::endl;
        output_stream << " reverseIndices = { ";

        auto it = reverseIndices.begin();
        if (it == reverseIndices.end()) {
            output_stream << "<empty>";
        } else {
            while (it != reverseIndices.end()) {
                output_stream << "   " << (*it).first << " => ";
                (*it).second->dumpToStream(output_stream);
                output_stream << std::endl;
                it++;
            }
        }

        output_stream << "}";
        output_stream.flush();
    }
}
