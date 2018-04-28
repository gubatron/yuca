#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include "types.hpp"
#include <map>
#include <set>
#include <memory>

namespace yuca {
    struct ReverseIndex {
    	ReverseIndex() : index(DocumentSet()) {}

        yuca::utils::Map<std::shared_ptr<Key>, DocumentSet> index;

        void putDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc);

        void removeDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc);

        auto hasDocuments(std::shared_ptr<Key> key) const -> bool;

	    DocumentSet getDocuments(std::shared_ptr<Key> key) const;

        long getKeyCount() const;

	    friend std::ostream& operator<<(std::ostream &output_stream, ReverseIndex &rindex);
    };

	struct TaggedKeywords {
		TaggedKeywords(std::string &query) : q(query), tag_keywords_map(yuca::utils::List<std::string>()) {
			std::string tag_prefix(":");
			yuca::utils::List<std::string> keywords = yuca::utils::split(query);
			std::string keyword_tag = ":keyword";
			std::string current_tag = ":keyword";
			for (auto keyword : keywords.getStdVector()) {
				if (yuca::utils::startsWith(keyword, tag_prefix)) {
					current_tag = keyword;
					continue;
				}
				auto tag_keywords = tag_keywords_map.get(current_tag);
				tag_keywords.add(keyword);
				tag_keywords_map.put(current_tag, tag_keywords);

				if (current_tag != keyword_tag) {
					auto keywords = tag_keywords_map.get(keyword_tag);
					keywords.add(keyword);
					tag_keywords_map.put(keyword_tag, keywords);
				}
			}
		}

		yuca::utils::List<std::string> getTags();

		yuca::utils::List<std::string> getKeywords(std::string &tag);

		yuca::utils::Map<std::string, yuca::utils::List<std::string>> tag_keywords_map;

		const std::string q;
	};

    class Indexer {
    public:
	    Indexer() : reverseIndices(std::shared_ptr<ReverseIndex>()) {}

        void indexDocument(std::shared_ptr<Document> doc);

        void removeDocument(std::shared_ptr<Document> doc);

        yuca::utils::List<std::shared_ptr<Document>> search(std::string &query) const;

        friend std::ostream& operator<<(std::ostream &output_stream, Indexer &indexer);

	    /** Given a key, it finds all related documents to its tag */
	    DocumentSet findDocuments(std::shared_ptr<Key> key) const;

	    DocumentSet findDocuments(int numKeys, std::shared_ptr<Key> keys[]) const;
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
        std::shared_ptr<ReverseIndex> getReverseIndex(std::string const &tag) const;

        void addToIndex(std::string const &tag, std::shared_ptr<Document> doc);

        yuca::utils::Map<std::string, std::shared_ptr<ReverseIndex>> reverseIndices;
    };
}

#endif //YUCA_INDEXER_HPP
