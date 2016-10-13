#include <iostream>

#include <yuca/version.hpp>
#include <yuca/indexer.hpp>

using namespace yuca;

int main() {
	std::cout << yuca::version() << std::endl;

	Document doc;
	Document::Key k;

	Indexer indexer;

	indexer.addDocument(std::move(doc));

	return 0;
}
