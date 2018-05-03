#include "indexer.hpp"
#include <iostream>

namespace yuca {

	void ReverseIndex::putDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc) {
		if (!hasDocuments(key)) {
			keyCachePut(key);
			SPDocumentSet newDocSet;
			newDocSet.add(doc);
			index.put(key, newDocSet);
		} else {
			auto cached_key = keyCacheGet(key);
			SPDocumentSet oldDocSet = getDocuments(cached_key);
			oldDocSet.add(doc);
			index.put(cached_key, oldDocSet);
		}
	}

	void ReverseIndex::removeDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc) {
		if (!hasDocuments(key)) {
			return;
		}
		auto cached_key = keyCacheGet(key);
		SPDocumentSet docs = index.get(cached_key);
		docs.remove(doc);
		index.remove(cached_key);

		if (!docs.isEmpty()) {
			index.put(cached_key, docs);
		}
	}

	auto ReverseIndex::hasDocuments(std::shared_ptr<Key> key) const -> bool {
		auto cached_key_sp = keyCacheGet(key);
		return index.containsKey(cached_key_sp);
	}

	std::shared_ptr<Key> ReverseIndex::keyCacheGet(std::shared_ptr<Key> key) const {
		return key_cache.get(key->getId());
	}

	void ReverseIndex::keyCachePut(std::shared_ptr<Key> key) {
		key_cache.put(key->getId(), key);
	}

	yuca::utils::List<std::string> SearchRequest::getTags() {
		return tag_keywords_map.keyList();
	}

	yuca::utils::List<OffsetKeyword> SearchRequest::getOffsetKeywords(std::string &tag) {
		return tag_keywords_map.get(tag);
	}

	SPDocumentSet ReverseIndex::getDocuments(std::shared_ptr<Key> key) const {
		if (!hasDocuments(key)) {
			return SPDocumentSet();
		}
		return index.get(keyCacheGet(key));
	}

	long ReverseIndex::getKeyCount() const {
		return index.size();
	}

	std::ostream &operator<<(std::ostream &output_stream, ReverseIndex &rindex) {
		output_stream << "ReverseIndex(@" << ((long) &rindex % 10000) << "):" << std::endl;
		auto it = rindex.index.getStdMap().begin();
		if (it == rindex.index.getStdMap().end()) {
			output_stream << "<empty>" << std::endl;
		} else {
			// index = { Key => SPDocumentSet }
			auto key_set = rindex.index.keySet();
			auto index_map = rindex.index.getStdMap();
			for (auto const& key_sp : key_set.getStdSet()) {
				output_stream << " ";
				output_stream << *key_sp;
				output_stream << " => ";
				auto docset = rindex.index.get(key_sp);
				output_stream << "(" << docset.size() << ") ";
				if (docset.isEmpty()) {
					output_stream << "<empty>" << std::endl;
				} else {
					output_stream << "[";
					auto docset_it = docset.getStdSet().begin();
					auto docset_it_end = docset.getStdSet().end();
					for (auto const& doc_sp : docset.getStdSet()) {
						output_stream << *doc_sp;
						docset_it++;
						if (docset_it != docset_it_end) {
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
		for (auto const &tag : tags) {
			addToIndex(tag, doc);
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
		std::set<std::string> tags = doc->getTags();
		for (auto const &tag : tags) {
			auto reverse_index = getReverseIndex(tag);
			SPKeySet key_set = doc->getTagKeys(tag);
			for (auto const &key : key_set.getStdSet()) {
				reverse_index->removeDocument(key, doc);
			}
		}
	}

	yuca::utils::List<SearchResult> Indexer::search(std::string &query, int max_search_results) const {
		SearchRequest search_request(query);

		// 1. Get a set  of Documents whose StringKey's match at least one of the
		// query keywords + corresponding tags as they come from the query string.
        SPDocumentSet spDocumentSet = findDocuments(search_request);

		// 2. Create search results and score them by the number of key hits
		yuca::utils::Map<std::shared_ptr<Document>, int> key_hits(0);
		for (auto const& doc_sp : spDocumentSet.getStdSet()) {
			for (auto tag : search_request.getTags().getStdVector()) {
				for (auto const& offset_keyword : search_request.getOffsetKeywords(tag).getStdVector()) {
					SPKeySet doc_sp_keyset = doc_sp->getTagKeys(tag);
					for (auto const& doc_key_sp : doc_sp_keyset.getStdSet()) {
						std::shared_ptr<StringKey> doc_string_key_sp = std::dynamic_pointer_cast<StringKey>(doc_key_sp);
						if (doc_string_key_sp->getString() == offset_keyword.keyword) {
							key_hits.put(doc_sp, key_hits.get(doc_sp) + 1);
							continue;
						}
					}
				}
			}
		}

		// 3. create search results and score them
		std::shared_ptr<SearchRequest> search_request_sp = std::make_shared<SearchRequest>(search_request);
		yuca::utils::List<SearchResult> results;
		for (auto const& doc_sp : spDocumentSet.getStdSet()) {
			SearchResult sr(search_request_sp, doc_sp);
			sr.score = key_hits.get(doc_sp) / (float) search_request.total_keywords;
			results.add(sr);
		}

		// TODO: Make score = w1.score1 + w2.score2 + ... + wN.scoreN

		// 4. sort list by score
		auto v = results.getStdVector();
		std::sort(v.begin(), v.end(), SearchResultSortFunctor());

		results.clear();

		for (auto const& sr : v) {
			results.add(sr);

			if (max_search_results > 0 && results.size() == max_search_results) {
				break;
			}
		}
		return results;
	}

	SPDocumentSet Indexer::findDocuments(SearchRequest &search_request) const {
		SPDocumentSet matchedSPDocSet;
		yuca::utils::List<std::string> search_tags = search_request.getTags();
		for (auto &tag : search_tags.getStdVector()) {
			yuca::utils::List<std::shared_ptr<Key>> string_keys;
			for (auto offset_keyword : search_request.getOffsetKeywords(tag).getStdVector()) {
				string_keys.add(std::make_shared<StringKey>(offset_keyword.keyword, tag));
			}
			matchedSPDocSet.addAll(findDocuments(string_keys));
		}
		return matchedSPDocSet;
	}

	SPDocumentSet Indexer::findDocuments(std::shared_ptr<Key> key) const {
		return getReverseIndex(key->getTag())->getDocuments(key);
	}

	SPDocumentSet Indexer::findDocuments(yuca::utils::List<std::shared_ptr<Key>> keys) const {
		SPDocumentSet docs_out;
		for (auto const &key : keys.getStdVector()) {
			SPDocumentSet docs;
			docs = findDocuments(key);
			if (!docs.isEmpty()) {
				for (auto const &doc_sp : docs.getStdSet()) {
					docs_out.add(doc_sp);
				}
			}
		}
		return docs_out;
	}

	void Indexer::addToIndex(std::string const &tag, std::shared_ptr<Document> doc) {
		SPKeySet doc_keys = doc->getTagKeys(tag);
		if (doc_keys.isEmpty()) {
			std::cout << "Indexer::addToIndex(" << tag
			          << "): check your logic, document has no doc_keys under this tag <"
			          << tag << ">" << std::endl;
			return;
		}
		// Make sure there's a ReverseIndex, if there isn't one, create an empty one
		std::shared_ptr<ReverseIndex> r_index = getReverseIndex(tag);
		if (r_index->getKeyCount() == 0) {
			reverseIndices.put(tag, std::make_shared<ReverseIndex>());
			r_index = getReverseIndex(tag);
		}
		for (auto const &k_sp : doc_keys.getStdSet()) {
			r_index->putDocument(k_sp, doc);
		}
		reverseIndices.put(tag, r_index);
	}

	std::shared_ptr<ReverseIndex> Indexer::getReverseIndex(std::string const &tag) const {
		if (!reverseIndices.containsKey(tag)) {
			return std::make_shared<ReverseIndex>();
		}
		return reverseIndices.get(tag);
	}

	std::ostream &operator<<(std::ostream &output_stream, Indexer &indexer) {
		output_stream << "Indexer(@" << ((long) &indexer % 10000) << "): " << std::endl;
		output_stream << " reverseIndices = { " << std::endl;

		if (indexer.reverseIndices.isEmpty()) {
			output_stream << "<empty>";
		} else {
			for (auto const& r_index_tag : indexer.reverseIndices.keySet().getStdSet()) {
				output_stream << "   " << r_index_tag << " => ";
				output_stream << *indexer.reverseIndices.get(r_index_tag);
				output_stream << std::endl;
			}
		}
		output_stream << "}";
		output_stream.flush();
		return output_stream;
	}
}
