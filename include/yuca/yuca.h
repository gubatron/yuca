//
// Created by gubatron on 11/19/16.
//

#ifndef YUCA_YUCA_H
#define YUCA_YUCA_H

#include <string>
#include <set>
#include <map>

namespace yuca {
    class Key;
    class Document;
    typedef std::string string;
    typedef std::set<Document *> DocumentSet;
    typedef std::set<Key *> KeySet;
    typedef std::map<std::string, KeySet> DocumentKeys;
}
#endif //YUCA_YUCA_H
