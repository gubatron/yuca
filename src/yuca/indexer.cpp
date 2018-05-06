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
		if (rindex.index.isEmpty()) {
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
			}
		}
		output_stream.flush();
		return output_stream;
	}

	void Indexer::indexDocument(Document doc) {
        SPDocument spDoc = std::make_shared<Document>(doc);
        indexDocument(spDoc);
	}

	void Indexer::removeDocument(Document doc) {
        SPDocument spDoc = docPtrCache.get(doc.getId());
        if (spDoc == nullptr) {
        	return;
        }
        removeDocument(spDoc);
	}

	void Indexer::indexDocument(SPDocument spDoc) {
		docPtrCache.put(spDoc->getId(), spDoc);

		std::set<std::string> tags = spDoc->getTags();
		for (auto const &tag : tags) {
			addToIndex(tag, spDoc);
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
		docPtrCache.remove(doc->getId());
	}

	void Indexer::clear() {
		if (docPtrCache.isEmpty()) {
			return;
		}
		SPDocumentList spList;
		for (auto docId : docPtrCache.keySet().getStdSetCopy()) {
			removeDocument(docPtrCache.get(docId));
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
		unsigned int num_tag_groups = (unsigned int) tags.size();
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

        // up to this point we have intersected (narrowed down) documents because they have matched
		// all the tags or groups specified in the search, now we need to see
		// why. How many of the given keywords in the search are matched by these guys.

		// 3. create search results and score them
		std::shared_ptr<SearchRequest> search_request_sp = std::make_shared<SearchRequest>(search_request);
		yuca::utils::List<SearchResult> results;
		std::set<std::string> search_tags = search_request.tag_keywords_map.keySet().getStdSetCopy();

		for (auto const& doc_sp : intersectedSPDocumentSet.getStdSet()) {
			std::cout << "Checking Document(" << doc_sp->stringProperty("full_name") << ") vs '" << search_request.query << "'" << std::endl;
			SearchResult sr(search_request_sp, doc_sp);
		    for (auto const& tag : search_tags) {
		    	std::cout << "\tUnder Tag " << tag << std::endl;
			    std::shared_ptr<ReverseIndex> tag_r_index = getReverseIndex(tag);
		    	SPKeySet spKeySet = doc_sp->getTagKeys(tag);
			    yuca::utils::List<OffsetKeyword> offset_search_keywords = search_request.tag_keywords_map.get(tag);
			    for (auto const& key : spKeySet.getStdSet()) {
			    	SPStringKey spStringKey = std::dynamic_pointer_cast<StringKey>(key);
			    	std::cout << "\t\tChecking Key <" << spStringKey->getString() << "> vs ";
			    	for (auto const& offset_keyword : offset_search_keywords.getStdVector()) {
			    		std::cout << " Keyword <" << offset_keyword.keyword << ">";
			    		StringKey mockup_string_key(offset_keyword.keyword, tag);
					    if (mockup_string_key.getId() == spStringKey->getId()) {
					    	std::cout << " => Doc(" << doc_sp->stringProperty("full_name") << ") matched Key " << mockup_string_key.getString() << mockup_string_key.getTag();
						    sr.score = sr.score + 1;
					    } else {
					    	std::cout << " => Nope! (keys don't match) " << mockup_string_key.getId() << " != " << spStringKey->getId();
					    }
					    std::cout << std::endl << "\t\t";
			    	}
			    	std::cout << std::endl;
			    }
		    }

			if (sr.score > 0) {
				results.add(sr);
			}
		}

		// TODO: Make score = w1.score1 + w2.score2 + ... + wN.scoreN

		// 4. sort list by score
		auto v = results.getStdVectorCopy();
		std::sort(v.begin(), v.end(), SearchResultSortFunctor());
		results.clear();
		for (auto const& result : v) {
			results.add(result);
		}
        if (max_search_results > 0) {
        	results = results.subList(0, max_search_results);
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
			yuca::utils::List<OffsetKeyword> tag_offset_keywords = search_request.getOffsetKeywords(tag);

			for (auto offset_keyword : tag_offset_keywords.getStdVector()) {
				search_spkey_list.add(std::make_shared<StringKey>(offset_keyword.keyword, tag));
			}
			SPDocumentSet tag_matched_spDoc_set = findDocuments(search_spkey_list);
			if (!tag_matched_spDoc_set.isEmpty()) {
				matchedSPDocSet.addAll(tag_matched_spDoc_set);
				r.put(tag, matchedSPDocSet);
			}
		}
		return r;
	}

	SPDocumentSet Indexer::findDocuments(SPKey key) const {
		//std::shared_ptr<ReverseIndex>spRIndex = getReverseIndex((key->getTag()));
		//SPDocumentSet results = spRIndex->getDocuments(key);
		//std::cout << "Indexer::findDocuments(keyId=" << key->getId() << ") [" << results.size() << "]" << std::endl;
		//return results;
		return getReverseIndex(key->getTag())->getDocuments(key);
	}

	SPDocumentSet Indexer::findDocuments(SPKeyList keys) const {
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
		output_stream << "{" << std::endl;
		output_stream << "\tdocPtrCache = { " << std::endl;
		auto doc_ids_set = indexer.docPtrCache.keySet();
		for (auto const& docId : doc_ids_set.getStdSet()) {
			SPDocument spDocument = indexer.docPtrCache.get(docId);
			if (spDocument != nullptr) {
				output_stream << "\t\t" << docId << " => " << *spDocument << std::endl;
			} else {
				std::cout << "WTF is the docId? " << docId << std::endl;
			}
		}
		output_stream << "\t}" << std::endl;
		output_stream << "\treverseIndices = { " << std::endl;
		if (indexer.reverseIndices.isEmpty()) {
			output_stream << "<empty>";
		} else {
			output_stream.flush();
			auto r_index_tags = indexer.reverseIndices.keySet();
			for (auto const& r_index_tag : r_index_tags.getStdSet()) {
				output_stream << "\t\t" << r_index_tag << " => ";
				output_stream << *indexer.reverseIndices.get(r_index_tag);
				output_stream << std::endl;
			}
		}
		output_stream << "\t}" << std::endl;
		output_stream << "}";
		output_stream.flush();
		return output_stream;
	}
}
