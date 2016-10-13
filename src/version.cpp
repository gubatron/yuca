#include "yuca/version.hpp"

#include <libtorrent/version.hpp>

namespace yuca {

	std::string  version() {
		return "yuca-0.0.1-" + std::string(libtorrent::version());
	}

}
