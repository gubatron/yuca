//
// Created by gubatron on 11/16/16.
//

#ifndef YUCA_KEY_H
#define YUCA_KEY_H

#endif //YUCA_KEY_H
namespace yuca {
    class Key {
    public:
        // gotta figure out how to make this a virtual method so implementing classes know they have to implement it
        bool operator<(const Key other) const {
            return true;
        }
    };
}
