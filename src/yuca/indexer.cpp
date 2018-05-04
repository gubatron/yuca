#include "indexer.hpp"
#include <iostream>

namespace yuca {

	void ReverseIndex::putDocument(SPKey key, SPDocument doc) {
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

	void ReverseIndex::removeDocument(SPKey key, SPDocument doc) {
		if (!hasDocuments(key)) {
			return;
		}
		auto cached_key = keyCacheGet(key);
		SPDocumentSet docs = index.get(cached_key);
		docs.remove(doc);

		if (!docs.isEmpty()) {
			index.put(cached_key, docs);
		} else {
			index.remove(cached_key);
			keyCacheRemove(cached_key);
		}
	}

	auto ReverseIndex::hasDocuments(SPKey key) const -> bool {
		auto cached_key_sp = keyCacheGet(key);
		return index.containsKey(cached_key_sp);
	}

	SPKey ReverseIndex::keyCacheGet(SPKey key) const {
		return keyPtrCache.get(key->getId());
	}

	void ReverseIndex::keyCachePut(SPKey key) {
		keyPtrCache.put(key->getId(), key);
	}

	void ReverseIndex::keyCacheRemove(SPKey key) {
		keyPtrCache.remove(key->getId());
	}


	yuca::utils::List<std::string> SearchRequest::getTags() {
		return tag_keywords_map.keyList();
	}

	yuca::utils::List<OffsetKeyword> SearchRequest::getOffsetKeywords(std::string &tag) {
		return tag_keywords_map.get(tag);
	}

	SPDocumentSet ReverseIndex::getDocuments(SPKey key) const {
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

	void Indexer::indexDocument(Document doc) {
        indexDocument(std::make_shared<Document>(doc));
	}

	void Indexer::removeDocument(Document doc) {

	}

	void Indexer::indexDocument(SPDocument doc) {
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

	void Indexer::removeDocument(SPDocument doc) {
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

		// 1. Get SETs of Documents (by tag) whose StringKey's match at least one of the
		// query keywords + corresponding tags as they come from the query string.
        yuca::utils::Map<std::string, SPDocumentSet> tagSPDocumentSetMap = findDocuments(search_request);

        // 1. Count document appearances per tag
		yuca::utils::Set<std::string> tags = tagSPDocumentSetMap.keySet();
		yuca::utils::Map<SPDocument, unsigned int> spDocs_appearances(0);
        for (auto const& tag : tags.getStdSet()) {
	        SPDocumentSet spDocumentSet = tagSPDocumentSetMap.get(tag);
	        // let's count doc appearances per tag as a way to intersect those that match in all asked tags
	        for (auto const &spDocument : spDocumentSet.getStdSet()) {
	        	spDocs_appearances.put(spDocument, 1 + spDocs_appearances.get(spDocument));
	        }
        }

        // filter out those documents that didn't meet the minimum number of appearances, that didn't appear
		// in all given tag groups.
		unsigned int num_tag_groups = tags.size();
		auto allSpDocsFound = spDocs_appearances.keySet().getStdSet();
		SPDocumentSet intersectedSPDocumentSet;

		if (num_tag_groups > 1) {
			for (auto const &spDoc : allSpDocsFound) {
				if (spDocs_appearances.get(spDoc) == num_tag_groups) {
					intersectedSPDocumentSet.add(spDoc);
				}
			}
		} else {
			intersectedSPDocumentSet = spDocs_appearances.keySet();
		}

		// Score them
		yuca::utils::Map<SPDocument, unsigned int> doc_key_hits(0);
		for (auto const& tag : tags.getStdSet()) {
			yuca::utils::List<OffsetKeyword> offsetKeywordList = search_request.tag_keywords_map.get(tag);
			// for each keyword in this tag group
			for (auto const& offsetKeyword : offsetKeywordList.getStdVector()) {
				// we ask each document if they have a key that matches the offsetKeyword from the query
				// Document's Tag's Key's memory addresses won't match if we search against a newly created SPStringKey
				// with the current keyword and tag, therefore we ask our reverse index cache to see if it has a copy
				// of a key like this one.
				std::string tag_copy = tag;
				std::string keyword_copy = offsetKeyword.keyword;
				StringKey searchStringKey(keyword_copy, tag_copy);
				SPStringKey searchSPStringKey = std::make_shared<StringKey>(searchStringKey);
				SPKey cachedSPKey = getReverseIndex(tag)->keyCacheGet(searchSPStringKey);

				if (cachedSPKey != nullptr) {
					for (auto const& spDocument : intersectedSPDocumentSet.getStdSet()) {
						SPKeySet spDocumentTagKeyset = spDocument->getTagKeys(tag);
						if (spDocumentTagKeyset.contains(cachedSPKey)) {
							doc_key_hits.put(spDocument, 1 + doc_key_hits.get(spDocument));
						}
					}
				}
			}
		}

		// 3. create search results and score them
		std::shared_ptr<SearchRequest> search_request_sp = std::make_shared<SearchRequest>(search_request);
		yuca::utils::List<SearchResult> results;
		for (auto const& doc_sp : intersectedSPDocumentSet.getStdSet()) {
			SearchResult sr(search_request_sp, doc_sp);
			sr.score = doc_key_hits.get(doc_sp) / (float) search_request.total_keywords;
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

	yuca::utils::Map<std::string, SPDocumentSet> Indexer::findDocuments(SearchRequest &search_request) const {
		SPDocumentSet emptyDocSet;
        yuca::utils::Map<std::string, SPDocumentSet> r(emptyDocSet);

		yuca::utils::List<std::string> search_tags = search_request.getTags();
		for (auto &tag : search_tags.getStdVector()) {
			SPDocumentSet matchedSPDocSet;
			SPKeyList search_spkey_list;
			for (auto offset_keyword : search_request.getOffsetKeywords(tag).getStdVector()) {
				search_spkey_list.add(std::make_shared<StringKey>(offset_keyword.keyword, tag));
			}
			matchedSPDocSet.addAll(findDocuments(search_spkey_list));
			r.put(tag, matchedSPDocSet);
		}
		return r;
	}

	SPDocumentSet Indexer::findDocuments(SPKey key) const {
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

	void Indexer::addToIndex(std::string const &tag, SPDocument doc) {
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
