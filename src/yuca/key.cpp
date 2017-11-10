//
// Created by gubatron on 11/16/16.
//

#include "key.hpp"
#include <iostream>

namespace yuca {
    bool Key::operator<(const Key other) const {
        std::cout << "Key::operator< : Comparing me(" << tag << ") vs other(" << other.tag << ")" << std::endl;
        std::cout.flush();
        auto myMemory = (long) this;
        auto otherMemoryOffset = (long) &other;
        return myMemory < otherMemoryOffset;
    }

    bool Key::operator==(const Key other) const {
        std::cout << "Key::operator== !" << std::endl;
        return (long) this == (long) &other;
    }

    std::string Key::getTag() const {
        return tag;
    }
}