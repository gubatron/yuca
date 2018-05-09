//
// Created by gubatron on 11/19/16.
//

#ifndef YUCA_YUCA_H
#define YUCA_YUCA_H
#include <string>
#include <yuca/indexer.hpp>

namespace yuca {
    typedef std::string string;

    std::string version() {
        //version: major.minor.build
        return "0.1.1";
    }
}
#endif //YUCA_YUCA_H
