#include <iostream>

#include <yuca/yuca.hpp>
#include <yuca/indexer.hpp>

using namespace yuca;

int main() {
	std::cout << yuca::version() << std::endl;
	Document doc;
	Indexer indexer;

    indexer.indexDocument(doc);
	return 0;
}
