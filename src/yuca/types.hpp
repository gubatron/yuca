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

    typedef std::set<Key *> KeySet;
    typedef std::set<Document *> DocumentSet;
}

#endif //YUCA_TYPES_HPP
