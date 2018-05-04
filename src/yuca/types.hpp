//
// Created by gubatron on 11/9/17.
//

#ifndef YUCA_TYPES_HPP
#define YUCA_TYPES_HPP

#include <memory>
#include <string>
#include <set>
#include "utils.hpp"

namespace yuca {
    class Key;
    class StringKey;
    class Document;

    typedef std::shared_ptr<Key> SPKey;
    typedef std::shared_ptr<StringKey> SPStringKey;
    typedef std::shared_ptr<Document> SPDocument;
    typedef yuca::utils::Set<SPKey> SPKeySet;
    typedef yuca::utils::List<SPKey> SPKeyList;
    typedef yuca::utils::Set<SPDocument> SPDocumentSet;
    typedef yuca::utils::List<SPDocument> SPDocumentList;
}

#endif //YUCA_TYPES_HPP
