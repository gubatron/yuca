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
// Created by gubatron on May/09/2018.
//

#include <iostream>

#include <yuca/yuca.hpp>

using namespace yuca;

void print(std::string const &query, yuca::utils::List<SearchResult> const &results);

int main() {
	std::cout << "Yuca v" << yuca::version() << std::endl;
	Document doc("some unique string identifier that will be hashed into a <long> value");

	// these can be added for convenience when documents are retrieved from search results
	// in case you want to have metadata associated to indexed documents, all property keys are
	// arbitrary and optionally defined by Yuca users to fit their indexing/retrieval needs.
	doc.longProperty("file_size", 2332898980);
	doc.boolProperty("top_secret", false);
	doc.intProperty("year", 1974);
	doc.stringProperty("author", "John Edgar Hoover");
	doc.stringProperty("title", "Masters of Deceit: The Story of Communism in America and How to Fight It");

	// Docs are indexed with Keys. Keys are grouped by ":tags".
	// There's a base class Key, and a provided StringKey implementation, you can implement
	// your own Keys for more advanced indexing needs, e.g. SizeRangeKey, LatitudeLongitudKey, etc.

	// Ideally you'd do the automatically with an algorithm in your program that extracts keys
	// given the object you want to index as a Document in the Yuca yuca.

	yuca::utils::List<StringKey> keys;
	// the "masters" keyword key, grouped by the ":keyword" tag
	// We'll tell the search engine our implicit/tagging group is ":keyword"
	keys.add(StringKey("masters", ":keyword"));
	keys.add(StringKey("deceit", ":keyword"));
	keys.add(StringKey("story", ":keyword"));
	keys.add(StringKey("communism", ":keyword"));
	keys.add(StringKey("america", ":keyword"));
	// if you wanted to do search by :author tag...
	keys.add(StringKey("hoover", ":author"));
	keys.add(StringKey("edgar", ":author"));
	// if you wanted to do searches by :year
	keys.add(StringKey("1974", ":year"));

	// Now we add the Keys to our Document
	for (StringKey const &key : keys.getStdVector()) {
		doc.addKey(key);
	}

	// Creates an index that uses the ":keyword" tag as the default implicit tag group
	// more on this below. Note: If you don't specify this in the constructor, the default
	// tag group used by the index will be by convention ":keyword" :)
	Indexer yuca(":keyword");
	yuca.indexDocument(doc); // you can index and remove documents at any time

	// If you're working in C++ you can work directly with shared pointers
	// we provide the interface above for easier integration with other languages
	// internally Yuca only keeps a single copy of every document and handles access
	// to them with std::shared_ptr
	// yuca.indexDocument(std::make_shared<Document>(doc));

	yuca::utils::List<SearchResult> results;

	// Once documents have been filtered and matched by tagged key groups
	// documents that match the same number of keys can be compared
	// to one of the stringProperties specified when Documents are entered
	// The document's whose string property is closest to the given query phrase
	// gets a higher ranking in the search result.
	// In this demo we'll use the "title" as the frame of reference.

	// If you don't want to compare against any property, this is optional, and you
	// can simply pass an empty string or use the convenience method yuca.search(queryHere);
	std::string title_property = "title";

	// We can limit the number of search results returned by passing an optional limit

	// simple query, by default it will search under our implicit default ":keyword" tag
	// search results will be those that match any of the given keywords, and they'll be ranked
	// by how similar they are to the given search query
	std::string query = "masters of deceit";
	results = yuca.search(query, title_property);
	print(query, results);

	// all documents matching any of the keys "edgar" or "hoover" filed under the ":author" tag
	query = ":author edgar hoover";
	results = yuca.search(query);
	print(query, results);

	// if we mix two or more tagged keyword groups, only documents that have matches
	// across in all tagged groups will be considered

	// the following search is equivalent to: ":keywords deceit masters communism :author edgar"
	// the first set of keywords will be considered to be under the explicit tag specified in
	// the Indexer's constructor
	query = "deceit masters communism :author edgar";
	results = yuca.search(query, title_property, 10);
	print(query, results);

	// These searches will fail

	// We never grouped documents under the tag ":title", if more than one tag group
	// is specified it means you want results filtered to meet all specified groups.
	query = ":title masters of deceit";
	results = yuca.search(query, title_property);
	print(query, results);

	// Even though it meets the keywords, it does not meet the author
	query = "deceit masters communism :author leon";
	results = yuca.search(query, title_property, 10);
	print(query, results);


	std::cout << std::endl << std::endl;
	// This is a string representation of the index in memory
	std::cout << yuca << std::endl;

	// You can remove documents from the index at any time
	// There's convenience versions of this method, you can pass the document id
	// or also a std::shared_ptr<Document>
	// You can empty the index by calling yuca.clear()
	yuca.removeDocument(doc);

    std::cout << std::endl << std::endl << "Empty Index:" << std::endl << std::endl;
    std::cout << yuca << std::endl << std::endl;

	return 0;
}

void print(std::string const &query, yuca::utils::List<SearchResult> const &results) {

	std::cout << "Query: " << query << std::endl;
	if (results.isEmpty()) {
		std::cout << "No search results." << std::endl << std::endl;
		return;
	}

	int i = 1;
	for (auto const &r : results.getStdVectorCopy()) {
		SPDocument spDoc = r.document_sp;
		std::string title = spDoc->stringProperty("title");
		std::string author = spDoc->stringProperty("author");
		int year = spDoc->intProperty("year");

		std::cout << i << ". " << title << std::endl;
		std::cout << "Author: " << author << std::endl;
		std::cout << "Year: " << year << std::endl;
		std::cout << "Document ID: " << spDoc->getId() << std::endl;
		std::cout << std::endl;
		i++;
	}

}