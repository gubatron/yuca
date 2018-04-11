//
// Created by gubatron on 11/16/16.
//

#ifndef YUCA_KEY_H
#define YUCA_KEY_H

#include <string>

namespace yuca {
    class Key {
    public:
        Key() = default;

        explicit Key(long id, std::string &my_tag) : id(id), tag(my_tag) {
        }

        // THIS OPERATOR IS USED FOR std::set.find()
        virtual bool operator<(const Key &right_side) const;

        virtual bool operator==(const Key &other) const;

        std::string getTag() const;

        long getId() const;

        friend std::ostream& operator<<(std::ostream &output_stream, Key &key);

    private:
        std::string tag;
        long id;
    };
}

#endif //YUCA_KEY_H

