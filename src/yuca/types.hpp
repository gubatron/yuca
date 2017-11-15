//
// Created by gubatron on 11/9/17.
//

#ifndef YUCA_TYPES_HPP
#define YUCA_TYPES_HPP

#include <string>
#include <set>

namespace yuca {
    class Key;
    class Document;

    typedef std::set<std::shared_ptr<Key>> KeySet;
    typedef std::set<std::shared_ptr<Document>> DocumentSet;
}

#endif //YUCA_TYPES_HPP
