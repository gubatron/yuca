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

std::string foo_tag(":foo");
std::string bar_tag(":bar");
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
		SPDocumentSet foo_key_docs = indexer.findDocuments(foo_key_sp);
		REQUIRE(foo_key_docs.size() == 1);

		// Look for a bar doc and find none.
		SPDocumentSet bar_key_docs = indexer.findDocuments(bar_key_sp);
		REQUIRE(bar_key_docs.size() == 0);

		// Look for the same doc using another key that also has a 'foo' tag
		SPDocumentSet foo_key2_docs = indexer.findDocuments(foo_key2_sp);
		REQUIRE(foo_key2_docs.size() == 1);

		// Index a document with 'bar' tagged key and find it
		indexer.indexDocument(document_bar_sp);
		bar_key_docs = indexer.findDocuments(bar_key_sp);
		REQUIRE(bar_key_docs.size() == 1);

		// search by the first key (fooKey)
		SPDocumentSet foo_found_docs = indexer.findDocuments(foo_key_sp);
		auto foo_it = foo_found_docs.getStdSet().begin();
		REQUIRE(foo_it != foo_found_docs.getStdSet().end()); // non-empty search results

		// Notes on shared_pointers.
		//   sp.get() -> pointer to object (*Object)
		//   *sp == *sp.get() -> dereferences the pointer to the object (Object)
		REQUIRE(**foo_it == *document_foo_sp);
		REQUIRE(false == (**foo_it == *document_bar_sp));

		SPDocumentSet bar_found_docs = indexer.findDocuments(bar_key_sp);
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
		SPDocumentSet foo_docs = indexer.findDocuments(foo_key_sp);
		REQUIRE(foo_docs.size() == 2);
		//////////////////////////////////////////////////////////////////

		SPDocumentSet bar_docs = indexer.findDocuments(bar_key_sp);
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

	    SPDocumentSet foo_docs = indexer.findDocuments(foo_key_sp);
	    SPDocumentSet bar_docs = indexer.findDocuments(bar_key_sp);

	    REQUIRE(foo_docs.size() == 2);
	    REQUIRE(bar_docs.size() == 2);

	    Indexer indexer_multi_key;
	    SPDocumentSet indexed_docs_checker;
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
	    List<std::shared_ptr<Key>> keys;
	    keys.add(foo_key_sp);
	    keys.add(bar_key_sp);
	    SPDocumentSet multi_index_doc_set = indexer_multi_key.findDocuments(keys);

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
//	    for (auto const& bar_doc : bar_docs.getStdSet()) {
//	    	std::cout << *bar_doc << std::endl;
//	    }

	    indexer_multi_key.removeDocument(document_bar_sp);
	    foo_docs = indexer_multi_key.findDocuments(foo_key_sp);
	    bar_docs = indexer_multi_key.findDocuments(bar_key_sp);
	    multi_index_doc_set = indexer_multi_key.findDocuments(keys);
	    REQUIRE(foo_docs.size() == 1);
	    REQUIRE(bar_docs.size() == 1);
	    REQUIRE(multi_index_doc_set.size() == 1);
    }
}

TEST_CASE("Indexer SearchRequest struct tests") {
	std::string keyword_tag(":keyword");
	std::string title_tag(":title");
	std::string extension_tag(":extension");
	std::string simple_query("simple search with no specific tags");
	std::string multi_tag_query(":title love is all you need :extension mp4");
	SearchRequest taggedKeywords(simple_query);

	List<std::string> tags = taggedKeywords.getTags();

	REQUIRE(tags.size() == 1);
	REQUIRE(tags.get(0) == keyword_tag);

	List<OffsetKeyword> keywords = taggedKeywords.getOffsetKeywords(keyword_tag);
	REQUIRE(keywords.get(0).keyword == "simple");
	REQUIRE(keywords.get(0).offset == 0);

	REQUIRE(keywords.get(1).keyword == "search");
	REQUIRE(keywords.get(1).offset == 7);

	REQUIRE(keywords.get(2).keyword == "with");
	REQUIRE(keywords.get(2).offset == 14);

	REQUIRE(keywords.get(3).keyword == "no");
	REQUIRE(keywords.get(3).offset == 19);

	REQUIRE(keywords.get(4).keyword == "specific");
	REQUIRE(keywords.get(4).offset == 22);

	REQUIRE(keywords.get(5).keyword == "tags");
	REQUIRE(keywords.get(5).offset == 31);

	SearchRequest multiTagKeywords(multi_tag_query);
	tags = multiTagKeywords.getTags();
	REQUIRE(tags.size() == 3);
	REQUIRE(tags.contains(":title"));
    REQUIRE(tags.contains(":extension"));
    REQUIRE(tags.contains(":keyword"));

    List<OffsetKeyword> title_keywords = multiTagKeywords.getOffsetKeywords(title_tag);
    REQUIRE(title_keywords.size() == 5);
    REQUIRE(title_keywords.get(0).keyword == "love");
	REQUIRE(title_keywords.get(1).keyword == "is");
	REQUIRE(title_keywords.get(2).keyword == "all");
	REQUIRE(title_keywords.get(3).keyword == "you");
	REQUIRE(title_keywords.get(4).keyword == "need");
}

/** generates a random integer in the interval [0,maxInclusive] */
struct file {
	std::string title;
	std::string ext;
	List<std::string> title_keywords;
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
			std::shared_ptr<StringKey> title_key = std::make_shared<StringKey>(title, title_tag);
			document_sp->addKey(title_key);

			std::string full_filename = full_name();
			std::shared_ptr<StringKey> filename_key = std::make_shared<StringKey>(full_filename, file_name_tag);
			document_sp->addKey(filename_key);

			std::string extension = ext;
			std::shared_ptr<StringKey> extension_key = std::make_shared<StringKey>(extension, extension_tag);
			document_sp->addKey(extension_key);

			for (long i = 0; i < title_keywords.size(); i++) {
				std::string token = title_keywords.get(i);
				std::shared_ptr<StringKey> keyword_key = std::make_shared<StringKey>(token, keyword_tag);
				document_sp->addKey(keyword_key);
			}
		}
		return document_sp;
	}
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

file generateRandomFile(const List<std::string> title_dict,
                        const List<std::string> ext_dict,
                        int min_words,
                        int max_words) {
	file f;
	List<std::string> phrase_n_tokens = generateRandomPhrase(title_dict, min_words + maxRand(max_words - min_words));
	f.title = phrase_n_tokens.get(0); // first element has full phrase as one string
	f.title_keywords = phrase_n_tokens.subList(1, phrase_n_tokens.size() - 1);
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

	int files = 10000;
	Indexer indexer;
	std::cout <<  std::endl << "Generating " << files << " random file names..." << std::endl;
	auto start = yuca::utils::timeInMillis();
	for (int i = 0; i < files; i++) {
		file f = generateRandomFile(title_dict, ext_dict, 4, 7);
		std::shared_ptr<Document> doc = f.get_document();
		doc->intProperty("id", i);
		doc->stringProperty("full_name",f.full_name());
		SPKeySet ext_keys = doc->getTagKeys(extension_tag);
		indexer.indexDocument(doc);
		if (i % 100 == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		//std::cout << i << ". [" << f.full_name() << "]" << std::endl << std::endl;
	}
	auto end = yuca::utils::timeInMillis();
	std::cout << std::endl << "Generated " << files << " random file names in " << (end-start) << "ms" << std::endl <<  std::endl;

	std::string q1("aquiles the polish :extension ogg");
	std::cout << "Searching for: <" << q1 << ">" << std::endl;
	std::shared_ptr<StringKey> cureKey = std::make_shared<StringKey>(q1,keyword_tag);
	//List<std::shared_ptr<SearchResult>> search_results_1 = indexer.search(q1);
	start = yuca::utils::timeInMillis();
	List<SearchResult> search_results_1 = indexer.search(q1, 10);
	end = yuca::utils::timeInMillis();

	auto duration = end - start;
	std::cout << std::endl << "==============================" << std::endl << std::endl;
	std::cout << "Found: " << search_results_1.size() << " docs searching for '" << q1 <<  "' in " << duration << "ms" << std::endl << std::endl;
	for (auto const& search_result_sp : search_results_1.getStdVector()) {
		std::cout << search_result_sp.document_sp->intProperty("id") << ". [" << search_result_sp.document_sp->stringProperty("full_name") << "], score: " << search_result_sp.score << std::endl;
	}
}

#endif
