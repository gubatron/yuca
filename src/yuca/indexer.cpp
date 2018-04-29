#include "indexer.hpp"
#include <iostream>

namespace yuca {

	void ReverseIndex::putDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc) {
		if (!hasDocuments(key)) {
			DocumentSet newDocSet;
			newDocSet.add(doc);
			keyCachePut(key);
			index.put(key, newDocSet);
		} else {
			DocumentSet oldDocSet = getDocuments(key);
			oldDocSet.add(doc);
			index.put(key, oldDocSet);
		}
	}

	void ReverseIndex::removeDocument(std::shared_ptr<Key> key, std::shared_ptr<Document> doc) {
		if (!hasDocuments(key)) {
			return;
		}

		DocumentSet docs = index.get(key);
		docs.remove(doc);
		index.remove(key);

		if (!docs.isEmpty()) {
			index.put(key, docs);
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

	yuca::utils::List<std::string> TaggedKeywords::getTags() {
		return tag_keywords_map.keyList();
	}

	yuca::utils::List<std::string> TaggedKeywords::getKeywords(std::string &tag) {
		return tag_keywords_map.get(tag);
	}

	DocumentSet ReverseIndex::getDocuments(std::shared_ptr<Key> key) const {
		if (!hasDocuments(key)) {
			return DocumentSet();
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
			// index = { Key => DocumentSet }
			while (it != rindex.index.getStdMap().end()) {
				output_stream << " ";
				output_stream << (*it).first;
				output_stream << " => ";
				auto docset_it = (*it).second.getStdSet().begin();
				output_stream << "(" << (*it).second.size() << ") ";
				if (docset_it == (*it).second.getStdSet().end()) {
					output_stream << "<empty>" << std::endl;
				} else {
					output_stream << "[";
					while (docset_it != (*it).second.getStdSet().end()) {
						output_stream << *docset_it;
						docset_it++;
						if (docset_it != (*it).second.getStdSet().end()) {
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
			KeySet key_set = doc->getTagKeys(tag);
			for (auto const &key : key_set.getStdSet()) {
				reverse_index->removeDocument(key, doc);
			}
		}
	}

	yuca::utils::List<std::shared_ptr<Document>> Indexer::search(std::string &query) const {
		yuca::utils::List<std::shared_ptr<Document>> results;
		TaggedKeywords taggedKeyword(query);

		yuca::utils::List<std::string> search_tags = taggedKeyword.getTags();
		for (auto &tag : search_tags.getStdVector()) {
			yuca::utils::List<std::shared_ptr<Key>> string_keys;
			for (auto keyword : taggedKeyword.getKeywords(tag).getStdVector()) {
				string_keys.add(std::make_shared<StringKey>(keyword, tag));
			}
			DocumentSet foundDocSet = findDocuments(string_keys);
			results.addAll(foundDocSet);
		}
		return results;
	}

	DocumentSet Indexer::findDocuments(std::shared_ptr<Key> key) const {
		return getReverseIndex(key->getTag())->getDocuments(key);
	}

	DocumentSet Indexer::findDocuments(yuca::utils::List<std::shared_ptr<Key>> keys) const {
		DocumentSet docs_out;
		for (auto const &key : keys.getStdVector()) {
			DocumentSet docs;
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
		KeySet doc_keys = doc->getTagKeys(tag);
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
		output_stream << " reverseIndices = { ";

		auto it = indexer.reverseIndices.getStdMap().begin();
		if (it == indexer.reverseIndices.getStdMap().end()) {
			output_stream << "<empty>";
		} else {
			while (it != indexer.reverseIndices.getStdMap().end()) {
				output_stream << "   " << (*it).first << " => ";
				output_stream << (*it).second; // second is the ReverseIndex
				output_stream << std::endl;
				it++;
			}
		}
		output_stream << "}";
		output_stream.flush();
		return output_stream;
	}
}
