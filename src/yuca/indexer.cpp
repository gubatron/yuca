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
            spkey_to_spdocset_map.put(key, newDocSet);
        } else {
            SPDocumentSet oldDocSet = getDocuments(key);
            oldDocSet.add(doc);
            auto cached_key = keyCacheGet(key);
            spkey_to_spdocset_map.put(cached_key, oldDocSet);
        }
    }

    void ReverseIndex::removeDocument(SPKey key, SPDocument doc) {
        if (!hasDocuments(key)) {
            std::cout << "ReverseIndex::removeDocument aborted. ReverseIndex has no documents under key "
                      << key->getId() << std::endl;
            return;
        }
        SPDocumentSet docs = getDocuments(key);
        SPKey cached_key = keyCacheGet(key);
        if (docs.isEmpty()) {
            spkey_to_spdocset_map.remove(cached_key);
            keyCacheRemove(cached_key);
        } else if (docs.contains(doc)) {
            docs.remove(doc);
            if (!docs.isEmpty()) {
                spkey_to_spdocset_map.put(cached_key, docs);
            } else {
                spkey_to_spdocset_map.remove(cached_key);
                keyCacheRemove(cached_key);
            }
        }
    }

    bool ReverseIndex::hasDocuments(SPKey key) const {
        auto cached_key_sp = keyCacheGet(key);
        if (cached_key_sp == nullptr) {
            return false;
        }
        return spkey_to_spdocset_map.containsKey(cached_key_sp) && spkey_to_spdocset_map.get(cached_key_sp).size() > 0;
    }

    void ReverseIndex::clear() {
        spkey_to_spdocset_map.clear();
        keyPtrCache.clear();
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

    yuca::utils::List<std::string> SearchRequest::getGroups() {
        return group_keywords_map.keyList();
    }

    yuca::utils::List<std::string> SearchRequest::getKeywords(std::string &group) {
        return group_keywords_map.get(group);
    }

    bool SearchRequest::operator==(const SearchRequest &other) const {
        return this->query == other.query && this->id == other.id && this->total_keywords == other.total_keywords;
    }

    bool SearchResult::operator==(const SearchResult &other) const {
        return id == other.id;
    }

    SPDocumentSet ReverseIndex::getDocuments(SPKey key) const {
        if (!hasDocuments(key)) {
            return SPDocumentSet();
        }
        return spkey_to_spdocset_map.get(keyCacheGet(key));
    }

    long ReverseIndex::getKeyCount() const {
        return static_cast<long>(spkey_to_spdocset_map.size());
    }

    std::ostream &operator<<(std::ostream &output_stream, ReverseIndex &rindex) {
        int truncated_address = (static_cast<int>((long) &rindex)) % 10000;
        output_stream << "ReverseIndex(@" << truncated_address << "):" << std::endl;
        if (rindex.spkey_to_spdocset_map.isEmpty()) {
            output_stream << "<empty>" << std::endl;
        } else {
            // spkey_to_spdocset_map = { Key => SPDocumentSet }
            auto key_set = rindex.spkey_to_spdocset_map.keySet();
            auto index_map = rindex.spkey_to_spdocset_map.getStdMap();
            for (auto const &key_sp : key_set.getStdSet()) {
                output_stream << " ";
                std::shared_ptr<StringKey> cast_key = std::dynamic_pointer_cast<StringKey>(key_sp);
                if (nullptr != cast_key) {
                    output_stream << *cast_key;
                } else {
                    output_stream << *key_sp;
                }
                output_stream << " => ";
                auto docset = rindex.spkey_to_spdocset_map.get(key_sp);
                output_stream << "(" << docset.size() << ") ";
                if (docset.isEmpty()) {
                    output_stream << "<empty>" << std::endl;
                } else {
                    output_stream << "[";
                    auto docset_it = docset.getStdSet().begin();
                    auto docset_it_end = docset.getStdSet().end();
                    for (auto const &doc_sp : docset.getStdSet()) {
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

    Document Indexer::getDocument(std::string const &doc_id) const noexcept {
        return getDocument(static_cast<long>(std::hash<std::string>{}(doc_id)));
    }

    bool Indexer::removeDocument(long const doc_id) {
        Document document = getDocument(doc_id);
        if (document == Document::NULL_DOCUMENT) {
            return false;
        }
        removeDocument(document);
        return true;
    }

    bool Indexer::removeDocument(std::string const &doc_id) {
        return removeDocument(static_cast<long>(std::hash<std::string>{}(doc_id)));
    }

    bool Indexer::removeDocument(Document doc) {
        SPDocument spDoc = docPtrCache.get(doc.getId());
        if (spDoc == nullptr) {
            return false;
        }
        removeDocument(spDoc);
        return true;
    }

    void Indexer::indexDocument(SPDocument spDoc) {
        docPtrCache.put(spDoc->getId(), spDoc);

        std::set<std::string> groups = spDoc->getGroups();
        for (auto const &group : groups) {
            addToIndex(group, spDoc);
        }
        // look for each one of the keys defined for this document
        // the keys come along with their group, which is used
        // by the indexer to partition the reverse indexes
        //
        // Document -> { groupName -> [ Key, Key2, ..., KeyN ], ... }
        //
        //
        // REVERSE_INDICES[group] ->
        //      \_
        //        [key1] = [Document1, Document2, ... ]
        //        [key2] = [Document1, Document2, ... ]
    }

    void Indexer::removeDocument(SPDocument doc) {
        std::set<std::string> groups = doc->getGroups();
        for (auto const &group : groups) {
            std::shared_ptr<ReverseIndex> reverse_index = getReverseIndex(group);

            SPStringKeySet key_set = doc->getGroupSPKeys(group);
            for (auto const &key : key_set.getStdSetCopy()) {
                reverse_index->removeDocument(key, doc);
            }
            if (reverse_index->getKeyCount() == 0) {
                reverse_index->clear();
                reverseIndices.remove(group);
            } else {
                reverseIndices.put(group, reverse_index);
            }
        }
        docPtrCache.remove(doc->getId());
    }

    void Indexer::clear() {
        for (auto const &group : reverseIndices.keySet().getStdSetCopy()) {
            auto spReverseIndex = getReverseIndex(group);
            spReverseIndex->clear();
        }
        reverseIndices.clear();
        docPtrCache.clear();
    }

    yuca::utils::List<SearchResult> Indexer::search(const std::string &query,
                                                    const std::string &opt_main_doc_property_for_query_comparison,
                                                    unsigned long opt_max_search_results) const {
        SearchRequest search_request(query, implicit_group);

        // 1. Get SETs of Documents (by group) whose StringKey's match at least one of the
        // query keywords + corresponding groups as they come from the query string.
        yuca::utils::Map<std::string, SPDocumentSet> groupSPDocumentSetMap = findDocuments(search_request);

        // 1. Count document appearances per group
        yuca::utils::Set<std::string> groups = groupSPDocumentSetMap.keySet();
        yuca::utils::Map<SPDocument, unsigned int> spDocs_appearances(0);
        for (auto const &group : groups.getStdSet()) {
            SPDocumentSet spDocumentSet = groupSPDocumentSetMap.get(group);
            // let's count doc appearances per group as a way to intersect those that match in all asked groups
            for (auto const &spDocument : spDocumentSet.getStdSet()) {
                spDocs_appearances.put(spDocument, 1 + spDocs_appearances.get(spDocument));
            }
        }

        // filter out those documents that didn't meet the minimum number of appearances, that didn't appear
        // in ALL given group groups.
        unsigned long num_group_groups = search_request.getGroups().size();
        auto allSpDocsFound = spDocs_appearances.keySet().getStdSet();
        SPDocumentSet intersectedSPDocumentSet;

        if (num_group_groups > 1) {
            for (auto const &spDoc : allSpDocsFound) {
                if (spDocs_appearances.get(spDoc) == num_group_groups) {
                    intersectedSPDocumentSet.add(spDoc);
                }
            }
        } else {
            intersectedSPDocumentSet = spDocs_appearances.keySet();
        }

        // Up to this point we have intersected (narrowed down) documents because they have matched
        // all the groups or groups specified in the search, now we need to see
        // why. How many of the given keywords in the search are matched by these guys.

        // 3. create search results and score them
        std::shared_ptr<SearchRequest> search_request_sp = std::make_shared<SearchRequest>(search_request);
        yuca::utils::List<SearchResult> results;
        std::set<std::string> search_groups = search_request.group_keywords_map.keySet().getStdSetCopy();

        for (auto const &doc_sp : intersectedSPDocumentSet.getStdSet()) {
            SearchResult sr(search_request_sp, doc_sp);

            for (auto const &group : search_groups) {
                std::shared_ptr<ReverseIndex> group_r_index = getReverseIndex(group);
                StringKeySet documentGroupKeys = doc_sp->getGroupKeys(group);
                yuca::utils::List<std::string> search_keywords = search_request.group_keywords_map.get(group);

                for (auto const &keyword : search_keywords.getStdVector()) {
                    StringKey searchStringKey(keyword, group);
                    if (documentGroupKeys.contains(searchStringKey)) {
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
                std::size_t LD = yuca::utils::levenshteinDistance(query, target_string);
                sr.score += LD == 0 ? 1.0 : 1 / LD;
            }
        }

        // 5. sort list by final score
        auto v = results.getStdVectorCopy();
        std::sort(v.begin(), v.end(), SearchResultSortFunctor());
        results.clear();
        for (auto const &result : v) {
            results.add(result);
        }
        if (opt_max_search_results > 0) {
            results = results.subList(0, opt_max_search_results);
        }
        return results;
    }

    yuca::utils::List<SearchResult> Indexer::search(const std::string &query) {
        return search(query, "", 0);
    }

    yuca::utils::List<SearchResult> Indexer::search(const std::string &query,
                                                    const std::string &opt_main_doc_property_for_query_comparison) {
        return search(query, opt_main_doc_property_for_query_comparison, 0);
    }

    yuca::utils::Map<std::string, SPDocumentSet> Indexer::findDocuments(SearchRequest &search_request) const {
        SPDocumentSet emptyDocSet;
        yuca::utils::Map<std::string, SPDocumentSet> r(emptyDocSet);

        yuca::utils::List<std::string> search_groups = search_request.getGroups();
        for (auto &group : search_groups.getStdVector()) {
            SPDocumentSet matchedSPDocSet;
            SPKeyList search_spkey_list;
            yuca::utils::List<std::string> keywords = search_request.getKeywords(group);

            for (auto keyword : keywords.getStdVector()) {
                search_spkey_list.add(std::make_shared<StringKey>(keyword, group));
            }
            SPDocumentSet group_matched_spDoc_set = findDocuments(search_spkey_list);
            if (!group_matched_spDoc_set.isEmpty()) {
                matchedSPDocSet.addAll(group_matched_spDoc_set);
                r.put(group, matchedSPDocSet);
            }
        }
        return r;
    }

    SPDocumentSet Indexer::findDocuments(SPKey key) const {
        // code remains in commented for step-by-step debugging purposes
        //std::shared_ptr<ReverseIndex>spRIndex = getReverseIndex((key->getGroup()));
        //SPDocumentSet results = spRIndex->getDocuments(key);
        //return results;
        return getReverseIndex(key->getGroup())->getDocuments(key);
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

    void Indexer::addToIndex(std::string const &group, SPDocument doc) {
        SPStringKeySet doc_keys = doc->getGroupSPKeys(group);
        if (doc_keys.isEmpty()) {
            std::cout << "Indexer::addToIndex(" << group
                      << "): check your logic, document has no doc_keys under this group <"
                      << group << ">" << std::endl;
            return;
        }
        // Make sure there's a ReverseIndex, if there isn't one, create an empty one
        std::shared_ptr<ReverseIndex> r_index = getReverseIndex(group);
        for (auto const &k_sp : doc_keys.getStdSet()) {
            r_index->putDocument(k_sp, doc);
        }
        reverseIndices.put(group, r_index);
    }

    std::shared_ptr<ReverseIndex> Indexer::getReverseIndex(std::string const &group) const {
        if (!reverseIndices.containsKey(group)) {
            return std::make_shared<ReverseIndex>();
        }
        return reverseIndices.get(group);
    }

    std::ostream &operator<<(std::ostream &output_stream, Indexer &indexer) {
        output_stream << "Indexer(@" << ((long) &indexer % 10000) << "): " << std::endl;
        output_stream << "{" << std::endl;
        output_stream << "\tdocPtrCache = { " << std::endl;
        auto doc_ids_set = indexer.docPtrCache.keySet();
        for (auto const &docId : doc_ids_set.getStdSet()) {
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
            auto r_index_groups = indexer.reverseIndices.keySet();
            for (auto const &r_index_group : r_index_groups.getStdSet()) {
                output_stream << "\t\t" << r_index_group << " => ";
                output_stream << *indexer.reverseIndices.get(r_index_group);
                output_stream << std::endl;
            }
        }
        output_stream << "\t}" << std::endl;
        output_stream << "}";
        output_stream.flush();
        return output_stream;
    }
}
