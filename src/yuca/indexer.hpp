#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include "types.hpp"
#include <map>
#include <set>
#include <memory>

namespace yuca {
	/** Maps *Key -> [Document Set] */
	struct ReverseIndex {
		ReverseIndex() : index(SPDocumentSet()), keyPtrCache(nullptr) {
		}

		yuca::utils::Map<std::shared_ptr<Key>, SPDocumentSet> index;

		void putDocument(SPKey key, SPDocument doc);

		void removeDocument(SPKey key, SPDocument doc);

		auto hasDocuments(SPKey key) const -> bool;

		SPDocumentSet getDocuments(SPKey key) const;

		long getKeyCount() const;

		/** Given an equivalent shared_ptr<Key> gets the corresponding shared_ptr<Key> we have stored already */
		SPKey keyCacheGet(SPKey) const;

		friend std::ostream &operator<<(std::ostream &output_stream, ReverseIndex &rindex);
	private:

		void keyCachePut(SPKey key);

		void keyCacheRemove(SPKey key);

		yuca::utils::Map<long, SPKey> keyPtrCache;
	};

	/**
	 * Represents a word and its offset with respect to a query phrase
	 */
	struct OffsetKeyword {
		OffsetKeyword(unsigned int pos, std::string &key_word) : offset(pos), keyword(key_word) {
		}

		unsigned int offset;
		std::string keyword;
	};

	/**
	 * Given a search query it creates a structure that maps
	 * possible search tags to specified OffsetKeywords
	 */
	struct SearchRequest {
		SearchRequest(std::string &query_str) :
		query(query_str),
		tag_keywords_map(yuca::utils::List<OffsetKeyword>()),
		id(rand()),
		total_keywords(0) {
			std::string tag_prefix(":");
			auto query_tokens = yuca::utils::split(query_str);
			std::string keyword_tag = ":keyword";
			std::string current_tag = ":keyword";
			unsigned int current_tag_offset = 0;
			for (auto keyword : query_tokens.getStdVector()) {
				if (yuca::utils::startsWith(keyword, tag_prefix)) {
					current_tag = keyword;
					current_tag_offset = query_str.find(current_tag) + current_tag.size();
					continue;
				}
				auto tag_keywords = tag_keywords_map.get(current_tag);
				OffsetKeyword offset_keyword((unsigned int) query_str.find(keyword, 0) - current_tag_offset, keyword);
				tag_keywords.add(offset_keyword);
				total_keywords++;
				tag_keywords_map.put(current_tag, tag_keywords);
				if (current_tag != keyword_tag) {
					auto keywords = tag_keywords_map.get(keyword_tag);
					keywords.add(offset_keyword);
					tag_keywords_map.put(keyword_tag, keywords);
				}
			}
		}

		yuca::utils::List<std::string> getTags();

		yuca::utils::List<OffsetKeyword> getOffsetKeywords(std::string &tag);

		yuca::utils::Map<std::string, yuca::utils::List<OffsetKeyword>> tag_keywords_map;

		const std::string query;

		const long id;

		long total_keywords;
	};

	struct SearchResult {
		SearchResult(std::shared_ptr<SearchRequest> searchRequest, SPDocument docSp) :
		search_request_sp(searchRequest),
		document_sp(docSp),
		id(docSp->getId() + searchRequest->id),
		score(0) {
		};

		float score; //[0.0 - 1.0]
		long id;
		std::shared_ptr<SearchRequest> search_request_sp;
		std::shared_ptr<Document> document_sp;
	};

	struct SearchResultSortFunctor {
		/** Use to sort SearchResults in descending order by score */
		bool operator()(const SearchResult a, const SearchResult b) const {
			return a.score > b.score;
		}
	};


	class Indexer {
	public:
		Indexer() : reverseIndices(std::shared_ptr<ReverseIndex>()), docPtrCache(nullptr) {
		}

		/** Wrapper meant for non C++ users so their API surface doesn't need to deal with shared_ptr */
		void indexDocument(Document doc);

		/** Wrapper meant for non C++ users so their API surface doesn't need to deal with shared_ptr */
		void removeDocument(Document doc);

		void indexDocument(SPDocument doc);

		void removeDocument(SPDocument doc);

		/** Remove all documents from the index */
		void clear();

		yuca::utils::List<SearchResult> search(std::string &query, int max_search_results) const;

		friend std::ostream &operator<<(std::ostream &output_stream, Indexer &indexer);

		yuca::utils::Map<std::string, SPDocumentSet> findDocuments(SearchRequest &search_request) const;

		/** Given a key, it finds all related documents to its tag */
		SPDocumentSet findDocuments(SPKey key) const;

		SPDocumentSet findDocuments(SPKeyList keys) const;
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

		void addToIndex(std::string const &tag, SPDocument doc);

		yuca::utils::Map<std::string, std::shared_ptr<ReverseIndex>> reverseIndices;

		yuca::utils::Map<long, SPDocument> docPtrCache;
	};
}

#endif //YUCA_INDEXER_HPP