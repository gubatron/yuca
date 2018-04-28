//
// Created by gubatron on 02/21/2017.
//

#ifndef YUCA_INDEXER_TESTS_H
#define YUCA_INDEXER_TESTS_H

// A Document can have many keys.
// Each key has to have a tag, which serves as a search dimension/partition parameter
// The indexer indexes documents with its keys.
// Documents store their own keys
// And the indexer keeps a reverse index which maps keys to sets of documents.

#include "tests_includes.hpp"

std::string foo_tag;
std::string bar_tag;
auto foo_key_sp = std::make_shared<Key>(1, foo_tag);
auto foo_key2_sp = std::make_shared<Key>(2, foo_tag);
auto bar_key_sp = std::make_shared<Key>(1, bar_tag);
std::shared_ptr<Document> document_foo_sp;
std::shared_ptr<Document> document_bar_sp;
std::shared_ptr<Document> document_foo_bar_sp;
std::string title_tag = ":title";
std::string file_name_tag = ":filename";
std::string keyword_tag = ":keyword";
std::string extension_tag = ":extension";


/**
 * We cache StringKeys.
 * map {
 *      :<tag> -> map {
 *                     :<word> -> StringKey(:<word>, :<tag>)
 *                    }
 *     }
 *
 */
std::string empty_string = "";
std::shared_ptr<StringKey> empty_string_key = std::make_shared<StringKey>(empty_string,empty_string);
yuca::utils::Map<std::string, std::shared_ptr<StringKey>> empty_word_to_StringKey_map(empty_string_key);
yuca::utils::Map<std::string, yuca::utils::Map<std::string, std::shared_ptr<StringKey>>>string_key_cache(empty_word_to_StringKey_map);

using namespace yuca::utils;

void initIndexerTests() {
	document_foo_sp = std::make_shared<Document>();
	document_foo_sp->addKey(foo_key_sp);
	document_foo_sp->addKey(foo_key2_sp);

	document_bar_sp = std::make_shared<Document>();
	document_bar_sp->addKey(bar_key_sp);

	document_foo_bar_sp = std::make_shared<Document>();
	document_foo_bar_sp->addKey(foo_key_sp);
	document_foo_bar_sp->addKey(bar_key_sp);
}

TEST_CASE("Indexer Basic Tests") {
	SECTION("Indexer.indexDocument") {
		initIndexerTests();

		// Index one doc with 'foo' tagged keys and find it.
		Indexer indexer;
		indexer.indexDocument(document_foo_sp);
		DocumentSet foo_key_docs = indexer.findDocuments(foo_key_sp);
		REQUIRE(foo_key_docs.size() == 1);

		// Look for a bar doc and find none.
		DocumentSet bar_key_docs = indexer.findDocuments(bar_key_sp);
		REQUIRE(bar_key_docs.size() == 0);

		// Look for the same doc using another key that also has a 'foo' tag
		DocumentSet foo_key2_docs = indexer.findDocuments(foo_key2_sp);
		REQUIRE(foo_key2_docs.size() == 1);

		// Index a document with 'bar' tagged key and find it
		indexer.indexDocument(document_bar_sp);
		bar_key_docs = indexer.findDocuments(bar_key_sp);
		REQUIRE(bar_key_docs.size() == 1);

		// search by the first key (fooKey)
		DocumentSet foo_found_docs = indexer.findDocuments(foo_key_sp);
		auto foo_it = foo_found_docs.getStdSet().begin();
		REQUIRE(foo_it != foo_found_docs.getStdSet().end()); // non-empty search results

		// Notes on shared_pointers.
		//   sp.get() -> pointer to object (*Object)
		//   *sp == *sp.get() -> dereferences the pointer to the object (Object)
		REQUIRE(**foo_it == *document_foo_sp);
		REQUIRE(false == (**foo_it == *document_bar_sp));

		DocumentSet bar_found_docs = indexer.findDocuments(bar_key_sp);
		REQUIRE(!bar_found_docs.isEmpty());
		auto bar_it = bar_found_docs.getStdSet().begin();
		REQUIRE(bar_it != bar_found_docs.getStdSet().end());

		Document &first_bar_found_doc = **bar_it;
		REQUIRE(first_bar_found_doc == *document_bar_sp);
		REQUIRE(false == (first_bar_found_doc == *document_foo_sp));

		// search by the 2nd key (fooKey2)
		foo_key2_docs = indexer.findDocuments(foo_key2_sp);
		Document &found_doc = **foo_key2_docs.getStdSet().begin();
		REQUIRE(found_doc == *document_foo_sp);

		// add foo and bar key doc, index should now return 2 results by both keys
		indexer.indexDocument(document_foo_bar_sp);
		DocumentSet foo_docs = indexer.findDocuments(foo_key_sp);
		REQUIRE(foo_docs.size() == 2);
		//////////////////////////////////////////////////////////////////

		DocumentSet bar_docs = indexer.findDocuments(bar_key_sp);
		// bar_docs = { document_bar_sp, document_foo_bar_sp }
		REQUIRE(bar_docs.size() == 2);
	}

	//////////////////////////////
	// multiple key search tests
	//////////////////////////////
    SECTION("MultiKey Indexing") {
	    initIndexerTests();

	    Indexer indexer;
	    indexer.indexDocument(document_foo_sp);
	    indexer.indexDocument(document_bar_sp);
	    indexer.indexDocument(document_foo_bar_sp);

	    DocumentSet foo_docs = indexer.findDocuments(foo_key_sp);
	    DocumentSet bar_docs = indexer.findDocuments(bar_key_sp);

	    REQUIRE(foo_docs.size() == 2);
	    REQUIRE(bar_docs.size() == 2);

	    Indexer indexer_multi_key;
	    DocumentSet indexed_docs_checker;
	    int n_docs_indexed = 0;

	    // add foo documents
	    for (auto sh_ptr_doc : foo_docs.getStdSet()) {
		    indexer_multi_key.indexDocument(sh_ptr_doc);
		    indexed_docs_checker.add(sh_ptr_doc);
	    }
	    for (auto sh_ptr_doc : bar_docs.getStdSet()) {
		    indexer_multi_key.indexDocument(sh_ptr_doc);
		    indexed_docs_checker.add(sh_ptr_doc);
	    }
	    n_docs_indexed = indexed_docs_checker.size();
	    std::shared_ptr<Key> keys[] = {foo_key_sp, bar_key_sp};
	    DocumentSet multi_index_doc_set = indexer_multi_key.findDocuments(2, keys);

	    REQUIRE(multi_index_doc_set.size() == n_docs_indexed);
	    REQUIRE(3 == n_docs_indexed);

	    foo_docs = indexer_multi_key.findDocuments(foo_key_sp);
	    bar_docs = indexer_multi_key.findDocuments(bar_key_sp);
	    REQUIRE(foo_docs.size() == 2);

	    REQUIRE(bar_docs.size() == 2);

	    indexer_multi_key.removeDocument(document_foo_sp);
	    foo_docs = indexer_multi_key.findDocuments(foo_key_sp);
	    bar_docs = indexer_multi_key.findDocuments(bar_key_sp);
	    REQUIRE(foo_docs.size() == 1);
	    REQUIRE(bar_docs.size() == 2);

	    indexer_multi_key.removeDocument(document_bar_sp);
	    foo_docs = indexer_multi_key.findDocuments(foo_key_sp);
	    bar_docs = indexer_multi_key.findDocuments(bar_key_sp);
	    multi_index_doc_set = indexer_multi_key.findDocuments(2, keys);
	    REQUIRE(foo_docs.size() == 1);
	    REQUIRE(bar_docs.size() == 1);
	    REQUIRE(multi_index_doc_set.size() == 1);
    }
}

/** We reuse the StringKey objects with shared_ptr's */
std::shared_ptr<StringKey> cache_get_StringKey(std::string &str, std::string &tag) {
	auto tag_word_map = string_key_cache.get(tag);
	if (tag_word_map == empty_word_to_StringKey_map) {
		Map<std::string, std::shared_ptr<StringKey>> word_to_stringKey_map(empty_string_key);
		auto result_sp = std::make_shared<StringKey>(str, tag);
		word_to_stringKey_map.put(str, result_sp);
		string_key_cache.put(tag, word_to_stringKey_map);
		return result_sp;
	} else {
		auto str_key_sp = tag_word_map.get(str);
		std::shared_ptr<StringKey> result_sp;
		if (str_key_sp == empty_string_key) {
			result_sp = std::make_shared<StringKey>(str, tag);
			tag_word_map.put(str, result_sp);
			string_key_cache.put(tag, tag_word_map);
			return result_sp;
		} else {
			return str_key_sp;
		}
	}
}

/** generates a random integer in the interval [0,maxInclusive] */
struct file {
	std::string title;
	std::string ext;
	List<std::string> title_tokens;
	std::shared_ptr<Document> document_sp = nullptr;
	std::string full_name() {
		std::string full;
		full.append(title);
		full.append(".");
		full.append(ext);
		return full;
	}
	std::shared_ptr<Document> get_document() {
		if (document_sp == nullptr) {
			document_sp = std::make_shared<Document>();
			std::shared_ptr<StringKey> title_key = cache_get_StringKey(title, title_tag);
			document_sp->addKey(title_key);

			std::string full_filename = full_name();
			std::shared_ptr<StringKey> filename_key = cache_get_StringKey(full_filename, file_name_tag);
			document_sp->addKey(filename_key);

			std::string extension = ext;
			std::shared_ptr<StringKey> extension_key = cache_get_StringKey(extension, extension_tag);
			document_sp->addKey(extension_key);

			for (long i = 0; i < title_tokens.size(); i++) {
				std::string token = title_tokens.get(i);
				std::shared_ptr<StringKey> token_key = cache_get_StringKey(token, keyword_tag);
				document_sp->addKey(token_key);
			}
		}
		return document_sp;
	}

	// todo: generate document out of file get_doc()
	// it should include all it's :title StringKeys and its :ext StringKey
};

/**
 * @param dictionary
 * @param words
 * @return The first element contains the entire phrase, the rest each word that helps make the phrase
 */
List<std::string> generateRandomPhrase(List<std::string> dictionary, int words) {
	std::string phrase;
	List<std::string> result;
	long max_index = dictionary.size() - 1;
	for (int i=0; i < words; i++) {
		std::string random_word = dictionary.get(yuca::utils::maxRand(max_index));
		result.add(random_word);
		phrase.append(random_word);

		if (i != words - 1) {
			phrase.append(" ");
		}
	}
	result.add(0, phrase);
	return result;
};

file generateRandomFile(List<std::string> title_dict,
                                   List<std::string> ext_dict,
                                   int min_words,
                                   int max_words) {
	file f;
	List<std::string> phrase_n_tokens = generateRandomPhrase(title_dict, min_words + maxRand(max_words - min_words));
	f.title = phrase_n_tokens.get(0);
	f.title_tokens = phrase_n_tokens.subList(1, phrase_n_tokens.size() - 1);
	f.ext = ext_dict.get(maxRand(static_cast<int>(ext_dict.size() - 1)));
    return f;
}

TEST_CASE("Indexer Search Tests") {
	List<std::string> title_dict;
	title_dict.add("troy");
	title_dict.add("paris");
	title_dict.add("hector");
	title_dict.add("aquiles");
	title_dict.add("helen");
	title_dict.add("agamenon");
	title_dict.add("apollo");
	title_dict.add("aphrodite");
	title_dict.add("xeon");
	title_dict.add("amazons");
	title_dict.add("dynamic");
	title_dict.add("polish");
	title_dict.add("yuca");
	title_dict.add("cure");
	title_dict.add("disease");
	title_dict.add("forest");
	title_dict.add("customs");
	title_dict.add("passport");
	title_dict.add("country");
	title_dict.add("airport");
	title_dict.add("plane");
	title_dict.add("boat");
	title_dict.add("car");
	title_dict.add("the");
	title_dict.add("it");
	title_dict.add("some");
	title_dict.add("love");
	title_dict.add("hate");
	title_dict.add("fear");
	title_dict.add("lullaby");
	title_dict.add("player");
	title_dict.add("locations");
	title_dict.add("cost");
	title_dict.add("cube");
    title_dict.add("rifle");

	List<std::string> ext_dict;
	ext_dict.add("txt");
	ext_dict.add("dat");
	ext_dict.add("gif");
	ext_dict.add("m4a");
	ext_dict.add("mp3");
	ext_dict.add("jpg");
	ext_dict.add("ogg");
	ext_dict.add("pdf");
	ext_dict.add("png");
	ext_dict.add("avi");

	std::srand(444);

	int queries = 100;
	Indexer indexer;
	for (int i = 0; i < queries; i++) {
		file f = generateRandomFile(title_dict, ext_dict, 4, 7);
		std::shared_ptr<Document> doc = f.get_document();
		indexer.indexDocument(doc);
		std::cout << *doc << std::endl;
		std::cout << i << ". [" << f.full_name() << "]" << std::endl << std::endl;
	}
}

#endif
