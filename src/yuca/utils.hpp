//
// Created by gubatron on 3/23/18.
//

#ifndef YUCA_UTILS_HPP
#define YUCA_UTILS_HPP

#include <set>
#include <map>

namespace yuca {
    namespace utils {
	    template<class T>
        class Set {
        public:
	        bool add(T e) {
                s.emplace(e);
                return true;
            }

            bool addAll(Set<T> other_set) {
                auto it = other_set.s.begin();
                while (it != other_set.s.end()) {
                    add(*it);
                    it++;
                }
                return true;
            }

            bool contains(T something) const noexcept {
                return s.find(something) != s.end();
            }

            bool isEmpty() const noexcept {
                return s.empty();
            }

            bool remove(T something) {
                auto it = s.find(something);
                if (it == s.end()) {
                    return false;
                }
                s.erase(it);
                return true;
            }

            bool removeAll(T other_set) {
                auto it = other_set.iterator();
                while (it != other_set.end()) {
                    remove(*it);
                    it++;
                }
                return true;
            }

            void copyTo(std::set<T> &other_std_set) const {
	        	other_std_set.clear();

	        	auto it = s.begin();
	        	while (it != s.end()) {
	        		other_std_set.insert(*it);
	        		it++;
	        	}
	        }

	        void dumpToStream(std::ostream &output_stream) const {
		        auto it = s.begin();
		        while (it != s.end()) {
			        output_stream << *it;
			        it++;
			        if (it != s.end()) {
				        output_stream << std::string(", ");
			        }
		        }
	        }

        private:
            std::set<T> s;
        };


        template<class K, class V>
        class Map {
        public:
            bool containsKey(K key) {
                return m.find(key) != m.end();
            }

            V get(K key) {
                auto it = m.find(key);
                if (it == m.end()) {
                    return nullptr;
                }
                return *it->second;
            }

        private:
            std::map<K, V> m;
        };
    }
}

#endif //YUCA_UTILS_HPP
