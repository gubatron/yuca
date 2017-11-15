#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include "types.hpp"
#include <map>
#include <set>

namespace yuca {

    struct ReverseIndex {
        std::map<std::shared_ptr<Key>, DocumentSet> index;

        void putDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc);

        bool hasDocuments(std::shared_ptr<Key> key) const;

        void getDocuments(std::shared_ptr<Key> key, DocumentSet &docs_out) const;

        long getKeyCount() const;

        void dumpToStream(std::ostream &output_stream) const;
    };

    class Indexer {
    public:
        void indexDocument(std::shared_ptr<Document> doc);

        void removeDocument(std::shared_ptr<Document> doc); // TODO

        void findDocuments(std::shared_ptr<Key> key, DocumentSet &docs_out) const;

        void findDocuments(int numKeys, std::shared_ptr<Key> keys[], DocumentSet &docs_out) const;

        void dumpToStream(std::ostream &output_stream) const;

    private:
        /**
         * The Indexer is conformed by multiple reverse indexes,
         * which are identified by a 'tag', this helps us partition our indexing
         * by categories.
         *
         * If there isn't a ReverseIndex for this tag, it'll create an empty one.
         *
         * For example, we could have a reverse index that catalogs documents by "file_extension"
         *
         * Inside the "file_extension" reverse index, we will find a multimap
         * of Key -> [Document, ...]
         *
         * Documents provide the indexer with the Keys to be used.
         *
         * */
        void getReverseIndex(std::string const &tag, std::shared_ptr<ReverseIndex> &r_index_out) const;

        void addToIndex(std::string const &tag, std::shared_ptr<Document> doc);

        std::map<std::string, std::shared_ptr<ReverseIndex>> reverseIndices;
    };
}

#endif //YUCA_INDEXER_HPP
