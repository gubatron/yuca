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

        virtual bool operator<=(const Key &right_side) const;

        virtual bool operator>(const Key &right_side) const;

        virtual bool operator>=(const Key &right_side) const;

        virtual bool operator==(const Key &other) const;

        std::string getTag() const;

        long getId() const;

        friend std::ostream& operator<<(std::ostream &output_stream, Key &key);

    protected:
        std::string tag;
        long id{};
    };

    class StringKey : public Key {
    public:
        explicit StringKey(std::string &string_key, std::string &my_tag) :
        Key(std::hash<std::string>{}(my_tag + string_key), my_tag), str_key(string_key) {
        }
        std::string getString() const {
            return str_key;
        }
        friend std::ostream& operator<<(std::ostream &output_stream, StringKey &key);
    private:
        std::string str_key;
    };
}

#endif //YUCA_KEY_H

