/**
 * MIT License
 *
 * Copyright (c) 2016-2018 Angel Leon, Alden Torres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// Created by gubatron on 11/16/16.
//

#ifndef YUCA_INDEXER_HPP
#define YUCA_INDEXER_HPP

#include "key.hpp"
#include "document.hpp"
#include "types.hpp"
#include <map>
#include <set>
#include <memory>
#include <algorithm>

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
		SearchRequest(const std::string &query_str, const std::string &implicit_tag) :
		query(query_str),
		tag_keywords_map(yuca::utils::List<OffsetKeyword>()),
		id(rand()),
		total_keywords(0) {
			std::string tag_prefix(":");
			std::string current_tag = implicit_tag;
			auto query_tokens = yuca::utils::split(query_str);
			unsigned int current_tag_offset = 0;
			for (auto& keyword : query_tokens.getStdVectorCopy()) {
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
		}

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
		Indexer(const std::string& implicit_tag) :
		implicit_tag(implicit_tag),
		reverseIndices(std::shared_ptr<ReverseIndex>()),
		docPtrCache(nullptr) {
		}

		Indexer() : Indexer(":keyword") {
		}

		/** Wrapper meant for non C++ users so their API surface doesn't need to deal with shared_ptr */
		void indexDocument(Document doc);

		Document getDocument(long doc_id) const noexcept ;

		Document getDocument(const std::string doc_id) const noexcept ;

		bool removeDocument(long doc_id);

		bool removeDocument(const std::string doc_id);

		/** Wrapper meant for non C++ users so their API surface doesn't need to deal with shared_ptr */
		void removeDocument(Document doc);

		void indexDocument(SPDocument doc);

		void removeDocument(SPDocument doc);

		/** Remove all documents from the index */
		void clear();

		/**
		 *
		 * @param query - Search string with support for :tagged keywords.
		 *
		 * If no :tags are specified, all keywords are grouped under the `:keyword` tag by default, meaning
		 * the user just gave a list of keywords to look for.
		 *
		 * Multiple <:tagged> key groups are expected in the following format
		 * ":tag1 t1_keyword_1 ... t1_keyword_N :tag2 t2_keyword_1 ... t2_keyword_N .... :tagN tN_keyword1 ... tN_keywordN
		 *
		 * "the twilight zone :extension mp4"
		 * ":city new york :year 2018 :sex female"
		 *
		 * @param opt_main_doc_property_for_query_comparison - Optional. Pass "" if you don't intend to use it.
		 * After tagged keys are used to filter out search results we can compare the given document property value
		 * against the given query. The lowest the Levenshtein distance the higher ranked the search result will be.
		 * If the given property name does not exist in the document, this parameter will simply be ignored.
		 *
		 * @param opt_max_search_results -  if > 0 it will limit the maximum search results to that number. Pass 0 or a
		 * negative number to obtain all possible search results.
		 *
		 * @return A List of matching SearchResult objects.
		 */
		yuca::utils::List<SearchResult> search(const std::string &query,
		                                       const std::string &opt_main_doc_property_for_query_comparison,
		                                       int opt_max_search_results) const;

        /**
         * Does not rank results by their resemblance to any Document property string key.
         * Does not limit the number of search results.
         * @param query
         * @return
         */
		yuca::utils::List<SearchResult> search(const std::string &query);

		/**
		 * Unlimited ranked search results by the given query doc string property
		 * @param query
		 * @param opt_main_doc_property_for_query_comparison
		 * @return
		 */
		yuca::utils::List<SearchResult> search(const std::string &query,
		                                       const std::string &opt_main_doc_property_for_query_comparison);

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

		const std::string implicit_tag;
	};
}

#endif //YUCA_INDEXER_HPP
