//
// Created by gubatron on 11/16/16.
//

#include "key.hpp"
#include <iostream>

namespace yuca {
    bool Key::operator<(const Key &right_side) const {
//        std::cout << "Key::operator< : Comparing me(" << tag << ":" << id << ") vs other(" << right_side.tag << ":" << right_side.id << ")" << std::endl;
//        std::cout << "result => " << (tag.compare(right_side.getTag()) < 0 && id < right_side.id) << std::endl;
//        std::cout.flush();
        return id < right_side.id;
    }

    bool Key::operator==(const Key &other) const {
//        std::cout << "Key::operator== : Comparing me(" << tag << ") vs other(" << other.tag << ")" << std::endl;
//        std::cout.flush();
        return id == other.id;
    }

    std::string Key::getTag() const {
        return tag;
    }

    long Key::getId() const {
        return id;
    }

    void Key::dumpToStream(std::ostream &output_stream) const {
        output_stream << "Key(id=" << id << ", tag=" << tag << ") @" << (long) this;
    }
}