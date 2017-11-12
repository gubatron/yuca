//
// Created by gubatron on 11/16/16.
//

#include "key.hpp"
#include <iostream>

namespace yuca {
    bool Key::operator<(const Key &rightSide) const {
        std::cout << "Key::operator< : Comparing me(" << tag << ":" << id << ") vs other(" << rightSide.tag << ":" << rightSide.id << ")" << std::endl;
        std::cout << "result => " << (tag.compare(rightSide.getTag()) < 0 && id < rightSide.id) << std::endl;
        std::cout.flush();
        return id < rightSide.id;
    }

    bool Key::operator==(const Key &other) const {
        std::cout << "Key::operator== : Comparing me(" << tag << ") vs other(" << other.tag << ")" << std::endl;
        std::cout.flush();
        return id == other.id;
    }

    std::string Key::getTag() const {
        return tag;
    }

    long Key::getId() const {
        return id;
    }
}