//
// Created by gubatron on 11/16/16.
//

#include "key.hpp"
#include <iostream>

namespace yuca {
    bool Key::operator<(const Key &right_side) const {
        return id < right_side.id ||
               (!(id > right_side.id));
    }

    bool Key::operator==(const Key &other) const {
        return id == other.id;
    }

    std::string Key::getTag() const {
        return tag;
    }

    long Key::getId() const {
        return id;
    }

    std::ostream &operator<<(std::ostream &output_stream, Key &key) {
        output_stream << "Key(@" << ((long) &key % 10000) << ", id=" << key.id << ", tag=" << key.tag << ")";
        output_stream.flush();
    }
}