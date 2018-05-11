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
// Created by gubatron.
//

#include "indexer.hpp"

namespace yuca {

	void ReverseIndex::putDocument(SPKey key, SPDocument doc) {
		if (!hasDocuments(key)) {
			keyCachePut(key);
			SPDocumentSet newDocSet;
			newDocSet.add(doc);
			sp_index_to_spdocset_map.put(key, newDocSet);
		} else {
			auto cached_key = keyCacheGet(key);
			SPDocumentSet oldDocSet = getDocuments(cached_key);
			oldDocSet.add(doc);
			sp_index_to_spdocset_map.put(cached_key, oldDocSet);
		}
	}

	void ReverseIndex::removeDocument(SPKey key, SPDocument doc) {

		if (!hasDocuments(key)) {
			return;
		}
		SPKey cached_key = keyCacheGet(key);
		SPDocumentSet docs = sp_index_to_spdocset_map.get(cached_key);
		if (docs.contains(doc)) {
			docs.remove(doc);

			if (!docs.isEmpty()) {
				sp_index_to_spdocset_map.put(cached_key, docs);
			} else {
				sp_index_to_spdocset_map.remove(cached_key);
				keyCacheRemove(cached_key);
			}
		}
	}

	auto ReverseIndex::hasDocuments(SPKey key) const -> bool {
		auto cached_key_sp = keyCacheGet(key);
		return sp_index_to_spdocset_map.containsKey(cached_key_sp);
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

	yuca::utils::List<std::string> SearchRequest::getKeywords(std::string &tag) {
		return tag_keywords_map.get(tag);
	}

	SPDocumentSet ReverseIndex::getDocuments(SPKey key) const {
		if (!hasDocuments(key)) {
			return SPDocumentSet();
		}
		return sp_index_to_spdocset_map.get(keyCacheGet(key));
	}

	long ReverseIndex::getKeyCount() const {
		return static_cast<long>(sp_index_to_spdocset_map.size());
	}

	std::ostream &operator<<(std::ostream &output_stream, ReverseIndex &rindex) {
		output_stream << "ReverseIndex(@" << ((long) &rindex % 10000) << "):" << std::endl;
		if (rindex.sp_index_to_spdocset_map.isEmpty()) {
			output_stream << "<empty>" << std::endl;
		} else {
			// sp_index_to_spdocset_map = { Key => SPDocumentSet }
			auto key_set = rindex.sp_index_to_spdocset_map.keySet();
			auto index_map = rindex.sp_index_to_spdocset_map.getStdMap();
			for (auto const& key_sp : key_set.getStdSet()) {
				output_stream << " ";
				std::shared_ptr<StringKey> cast_key = std::dynamic_pointer_cast<StringKey>(key_sp);
				if (nullptr != cast_key) {
				    output_stream << *cast_key;
				} else {
					output_stream << *key_sp;
				}
				output_stream << " => ";
				auto docset = rindex.sp_index_to_spdocset_map.get(key_sp);
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

	Document Indexer::getDocument(long doc_id) const noexcept {
		SPDocument spDocument = docPtrCache.get(doc_id);
		if (spDocument == nullptr) {
			return Document::NULL_DOCUMENT;
		}
		return *spDocument;
	}

	Document Indexer::getDocument(std::string const& doc_id) const noexcept {
        return getDocument(std::hash<std::string>{}(doc_id));
	}

	bool Indexer::removeDocument(long const doc_id) {
        Document document = getDocument(doc_id);
        if (document == Document::NULL_DOCUMENT) {
        	return false;
        }
        removeDocument(document);
        return true;
	}

	bool Indexer::removeDocument(std::string const& doc_id) {
        return removeDocument(std::hash<std::string>{}(doc_id));
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
			std::shared_ptr<ReverseIndex> reverse_index = getReverseIndex(tag);
			SPKeySet key_set = doc->getTagSPKeys(tag);
			for (auto const &key : key_set.getStdSetCopy()) {
				reverse_index->removeDocument(key, doc);
			}
			if (reverse_index->getKeyCount() == 0) {
				reverseIndices.remove(tag);

			}
		}
		docPtrCache.remove(doc->getId());
	}

	void Indexer::clear() {
		if (docPtrCache.isEmpty()) {
			//return;
		}
		SPDocumentList spList;
		for (auto &docId : docPtrCache.keySet().getStdSetCopy()) {
			SPDocument spDocument = docPtrCache.get(docId);
			std::cout << "Indexer::clear. Remove doc -> " << spDocument->getId() << std::endl;
			removeDocument(spDocument);
		}
	}

	yuca::utils::List<SearchResult> Indexer::search(const std::string &query,
	                                                const std::string &opt_main_doc_property_for_query_comparison,
	                                                int opt_max_search_results) const {
		SearchRequest search_request(query, implicit_tag);

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
		// in ALL given tag groups.
		unsigned int num_tag_groups = (unsigned int) search_request.getTags().size();
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

        // Up to this point we have intersected (narrowed down) documents because they have matched
		// all the tags or groups specified in the search, now we need to see
		// why. How many of the given keywords in the search are matched by these guys.

		// 3. create search results and score them
		std::shared_ptr<SearchRequest> search_request_sp = std::make_shared<SearchRequest>(search_request);
		yuca::utils::List<SearchResult> results;
		std::set<std::string> search_tags = search_request.tag_keywords_map.keySet().getStdSetCopy();

		for (auto const& doc_sp : intersectedSPDocumentSet.getStdSet()) {
			SearchResult sr(search_request_sp, doc_sp);

		    for (auto const& tag : search_tags) {
			    std::shared_ptr<ReverseIndex> tag_r_index = getReverseIndex(tag);
		    	KeySet documentTagKeys = doc_sp->getTagKeys(tag);
			    yuca::utils::List<std::string> search_keywords = search_request.tag_keywords_map.get(tag);

			    for (auto const& keyword : search_keywords.getStdVector()) {
			    	StringKey searchStringKey(keyword, tag);
			    	if (documentTagKeys.contains(searchStringKey)) {
			    		sr.score++;
			    	}
			    }
		    }

			if (sr.score > 0) {
				results.add(sr);
			}
		}

		// 4. if we were given a valid document property name to perform a Levenshtein distance
		// calculation we'll try to rank results by scoring higher those that get the lowest
		if (opt_main_doc_property_for_query_comparison.length() > 0) {
            for (auto &sr : results.getStdVector()) {
            	std::string target_string = sr.document_sp->stringProperty(opt_main_doc_property_for_query_comparison);
            	if (target_string.length() == 0) {
            		continue;
            	}
                unsigned int LD = yuca::utils::levenshteinDistance(query, target_string);
	            sr.score += LD == 0 ? 1.0 : 1 / (float) LD;
            }
		}

		// 5. sort list by final score
		auto v = results.getStdVectorCopy();
		std::sort(v.begin(), v.end(), SearchResultSortFunctor());
		results.clear();
		for (auto const& result : v) {
			results.add(result);
		}
        if (opt_max_search_results > 0) {
        	results = results.subList(0, opt_max_search_results);
        }
		return results;
	}

	yuca::utils::List<SearchResult> Indexer::search(const std::string &query) {
		return search(query, "", -1);
	}

	yuca::utils::List<SearchResult> Indexer::search(const std::string &query,
	                                       const std::string &opt_main_doc_property_for_query_comparison) {
		return search(query, opt_main_doc_property_for_query_comparison, -1);
	}

	yuca::utils::Map<std::string, SPDocumentSet> Indexer::findDocuments(SearchRequest &search_request) const {
		SPDocumentSet emptyDocSet;
        yuca::utils::Map<std::string, SPDocumentSet> r(emptyDocSet);

		yuca::utils::List<std::string> search_tags = search_request.getTags();
		for (auto &tag : search_tags.getStdVector()) {
			SPDocumentSet matchedSPDocSet;
			SPKeyList search_spkey_list;
			yuca::utils::List<std::string> keywords = search_request.getKeywords(tag);

			for (auto keyword : keywords.getStdVector()) {
				search_spkey_list.add(std::make_shared<StringKey>(keyword, tag));
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
		// code remains in commented for step-by-step debugging purposes
		//std::shared_ptr<ReverseIndex>spRIndex = getReverseIndex((key->getTag()));
		//SPDocumentSet results = spRIndex->getDocuments(key);
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
		SPKeySet doc_keys = doc->getTagSPKeys(tag);
		if (doc_keys.isEmpty()) {
			std::cout << "Indexer::addToIndex(" << tag
			          << "): check your logic, document has no doc_keys under this tag <"
			          << tag << ">" << std::endl;
			return;
		}
		// Make sure there's a ReverseIndex, if there isn't one, create an empty one
		std::shared_ptr<ReverseIndex> r_index = getReverseIndex(tag);
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
