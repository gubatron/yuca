//
// Created by gubatron on 11/16/16.
//

#ifndef YUCA_KEY_H
#define YUCA_KEY_H

#endif //YUCA_KEY_H
namespace yuca {
    class Key {
    public:
        Key() {}

        Key(std::string myTag) : tag(myTag) {}

        // gotta figure out how to make this a virtual method so implementing classes know they have to implement it
        bool operator<(const Key other) const {
            std::cout << "Key::operator< : Comparing me(" << tag << ") vs other(" << other.tag << ")" << std::endl;
            std::cout.flush();
            long myMemory = (long) this;
            long otherMemoryOffset = (long) &other;
            return myMemory < otherMemoryOffset;
        }

        bool operator==(const Key other) const {
            std::cout << "Key::operator== !" << std::endl;
            return (long) this == (long) &other;
        }

        std::string getTag()const {
            return tag;
        }

    private:
        std::string tag;
    };
}
