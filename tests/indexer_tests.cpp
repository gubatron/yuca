//
// Created by gubatron on 02/21/2017.
//

#ifndef YUCA_INDEXER_TESTS_H
#define YUCA_INDEXER_TESTS_H

#include <yuca/indexer.hpp>

// A Document can have many keys.
// Each key has to have a tag, which serves as a search dimension/partition parameter
// The indexer indexes documents with its keys.
// Documents store their own keys
// And the indexer keeps a reverse index which maps keys to sets of documents.

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

TEST_CASE("Test Indexer.indexDocument") {
    initIndexerTests();

    // Index one doc with 'foo' tagged keys and find it.
    Indexer indexer;
    indexer.indexDocument(document_foo_sp);
    DocumentSet foo_key_docs;
    indexer.findDocuments(foo_key_sp, foo_key_docs);
    REQUIRE(foo_key_docs.size() == 1);

    // Look for a bar doc and find none.
    DocumentSet bar_key_docs;
    indexer.findDocuments(bar_key_sp, bar_key_docs);
    REQUIRE(bar_key_docs.size() == 0);

    // Look for the same doc using another key that also has a 'foo' tag
    DocumentSet foo_key2_docs;
    indexer.findDocuments(foo_key2_sp, foo_key2_docs);
    REQUIRE(foo_key2_docs.size() == 1);

    // Index a document with 'bar' tagged key and find it
    indexer.indexDocument(document_bar_sp);
    indexer.findDocuments(bar_key_sp, bar_key_docs);
    REQUIRE(bar_key_docs.size() == 1);

    // search by the first key (fooKey)
    DocumentSet foo_found_docs;
    indexer.findDocuments(foo_key_sp, foo_found_docs);
    auto foo_it = foo_found_docs.begin();
    REQUIRE(foo_it != foo_found_docs.end()); // non-empty search results
//	std::cout << "**foo_it -> " << std::endl << **foo_it << std::endl;
//	std::cout << "*document_foo_sp.get() -> " << std::endl << *document_foo_sp.get() << std::endl;
    REQUIRE(**foo_it == *document_foo_sp.get());
    REQUIRE(false == (**foo_it == *document_bar_sp.get()));

    DocumentSet bar_found_docs;
    indexer.findDocuments(bar_key_sp, bar_found_docs);
    DocumentSet::iterator bar_it = bar_found_docs.begin();
    REQUIRE(bar_it != bar_found_docs.end());

    Document &first_bar_found_doc = **bar_it;
    REQUIRE(first_bar_found_doc == *document_bar_sp);
    REQUIRE(false == (first_bar_found_doc == *document_foo_sp));

    // search by the 2nd key (fooKey2)
    indexer.findDocuments(foo_key2_sp, foo_key2_docs);
    Document &found_doc = **foo_key2_docs.begin();
    REQUIRE(found_doc == *document_foo_sp);

    // add foo and bar key doc, index should now return 2 results by both keys

    indexer.indexDocument(document_foo_bar_sp);
    DocumentSet foo_docs;
    indexer.findDocuments(foo_key_sp, foo_docs);
    REQUIRE(foo_docs.size() == 2);

    foo_it = foo_docs.begin();
	REQUIRE(**foo_it == *document_foo_bar_sp.get()); // Document == Document
	REQUIRE((*foo_it) == document_foo_bar_sp); // shared_ptr<Document> == shared_ptr<Document>

	foo_it++;

	// Notes on shared_pointers.
	//   sp.get() -> pointer to object (*Object)
	//   *sp.get() -> dereferences the pointer to the object (Object)
	REQUIRE(**foo_it == *document_foo_sp.get()); // Document == Document
	REQUIRE((*foo_it) == document_foo_sp); // shared_ptr<Document> == shared_ptr<Document>

	foo_it++;

	REQUIRE(foo_it == foo_docs.end());
	//////////////////////////////////////////////////////////////////

	DocumentSet bar_docs;
	indexer.findDocuments(bar_key_sp, bar_docs);
	// bar_docs = { document_bar_sp, document_foo_bar_sp }
	REQUIRE(bar_docs.size() == 2);
	bar_it = bar_docs.begin();
	REQUIRE(**bar_it == *document_bar_sp.get()); // bar Document == bar Document
    bar_it++;
	REQUIRE(**bar_it == *document_foo_bar_sp.get()); // foo bar Document == foo bar Document
    bar_it++;
    REQUIRE(bar_it == bar_docs.end());

    //////////////////////////////
    // multiple key search tests
    //////////////////////////////

    Indexer indexer_multi_key;
    foo_it = foo_docs.begin();
    bar_it = bar_docs.begin();
    DocumentSet indexed_docs_checker;
    int n_docs_indexed = 0;

    // add foo documents
    while (foo_it != foo_docs.end()) {
        indexer_multi_key.indexDocument(*foo_it);
        indexed_docs_checker.insert(*foo_it);
        //std::cout << **foo_it;
        foo_it++;
        n_docs_indexed = indexed_docs_checker.size();
    }
    while (bar_it != bar_docs.end()) {
        indexer_multi_key.indexDocument(*bar_it);
        indexed_docs_checker.insert(*bar_it);
        //std::cout << **bar_it;
        bar_it++;
        n_docs_indexed = indexed_docs_checker.size();
    }
    std::shared_ptr<Key> keys[] = { foo_key_sp, bar_key_sp };
    DocumentSet multi_index_doc_set;
    indexer_multi_key.findDocuments(2, keys, multi_index_doc_set);
    //std::cout << "Docs found: " << multi_index_doc_set.size();

    REQUIRE(multi_index_doc_set.size() == n_docs_indexed);
}

#endif
