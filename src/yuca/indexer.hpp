#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include "types.hpp"
#include <map>
#include <set>

namespace yuca {

    struct ReverseIndex {
        std::map<Key, DocumentSet> index;

        void putDocument(Key const &key, Document const &doc);

        bool hasDocuments(Key const &key) const;

        void getDocuments(Key const &key, DocumentSet &docs_out) const;

        long getKeyCount() const;

        void dumpToStream(std::ostream &output_stream) const;
    };

    class Indexer {
    public:
        void indexDocument(Document const &doc);

        void removeDocument(Document const &doc); // TODO

        void findDocuments(Key const &key, DocumentSet &docs_out) const;

        void findDocuments(int numKeys, Key keys[], DocumentSet &docs_out) const;

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
        void getReverseIndex(std::string const &tag, ReverseIndex &r_index_out) const;

        void addToIndex(std::string const &tag, Document const &doc);

        std::map<std::string, ReverseIndex> reverseIndices;
    };
}

#endif //YUCA_INDEXER_HPP
