//
// Created by gubatron on 11/16/16.
//

#ifndef YUCA_KEY_H
#define YUCA_KEY_H

#include <string>
#include <utility>

namespace yuca {
    class Key {
    public:
        Key() = default;

        explicit Key(long id, std::string myTag) : tag(std::move(myTag)), id(id) {
        }

        explicit Key(std::string myTag) : tag(std::move(myTag)), id(rand()) {
        }

        // THIS OPERATOR IS USED FOR std::set.find()
        virtual bool operator<(const Key &rightSide) const;

        virtual bool operator==(const Key &other) const;

        std::string getTag() const;

        long getId() const;

    private:
        std::string tag;
        long id;
    };
}

#endif //YUCA_KEY_H

