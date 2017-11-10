#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include "types.hpp"
#include <map>
#include <set>

namespace yuca {

    struct ReverseIndex {
        std::map<Key *, DocumentSet> index;

        void putDocument(Key *key, Document *doc);

        bool hasDocuments(Key *key) const;

        DocumentSet getDocuments(Key *key) const;

        long getKeyCount() const;
    };

    class Indexer {
    public:
        void indexDocument(Document const &doc);

        void removeDocument(Document const &doc); // TODO
        DocumentSet findDocuments(Key &key) const;

        DocumentSet findDocuments(int numKeys, Key keys[]) const;

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
        ReverseIndex *const getReverseIndex(std::string const &tag) const;

        void addToIndex(std::string const &tag, const Document &doc);

        std::map<std::string, ReverseIndex *> reverseIndices;
    };
}

#endif //YUCA_INDEXER_HPP
