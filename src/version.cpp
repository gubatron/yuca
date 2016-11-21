//
// Created by gubatron on 11/20/16.
//
#include <libtorrent/version.hpp>
#include <ios>

namespace yuca {

	std::string  version() {
		return "yuca-0.0.1-" + std::string(libtorrent::version());
	}

}
