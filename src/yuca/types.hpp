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
    class Document;

    typedef yuca::utils::Set<std::shared_ptr<Key>> KeySet;
    typedef yuca::utils::Set<std::shared_ptr<Document>> DocumentSet;
}

#endif //YUCA_TYPES_HPP
