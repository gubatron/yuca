/**
 * MIT License
 *
 * Copyright (c) 2016-2018 Angel Leon, Alden Torres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// Java Collections Inspired wrappers for std:: collections
// Created by gubatron on 3/23/18.
//

#ifndef YUCA_UTILS_HPP
#define YUCA_UTILS_HPP

#include <set>
#include <map>
#include <vector>
#include <ostream>
#include <iostream>
#include <sstream>
#include <numeric>
#include <chrono>
#include <algorithm>

namespace yuca {
    namespace utils {
        const std::string VERSION = "1.0.2";
        const std::string LAST_UPDATED = "2018/04/05"; // YYYY/MM/DD

        template<class T>
        class Set {
        public:
            void add(T const &e) {
                s.insert(e);
            }

            void addAll(Set<T> &other_set) noexcept {
                for (auto const &x : other_set.s) {
                    add(x);
                }
            }

            bool contains(T const &something) const noexcept {
                return s.find(something) != s.end();
            }

            bool containsAll(Set<T> &other) const noexcept {
                for (auto const &x : other.s) {
                    if (!contains(x)) {
                        return false;
                    }
                }
                return true;
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

            void removeAll(Set<T> &other_set) {
                for (auto const &x : other_set.s) {
                    remove(x);
                }
            }

            void clear() noexcept {
                s.clear();
            }

            /**
             * @return a copied std::set<T> instance of the containing std::set<T> s
             */
            std::set<T> getStdSetCopy() const noexcept {
                return s;
            }

            std::set<T> &getStdSet() noexcept {
                return s;
            }

            unsigned long size() const noexcept {
                return s.size();
            }

            friend std::ostream &operator<<(std::ostream &output_stream, Set<T> set) {
                long last_index = set.size() - 1;
                long i = 0;
                output_stream << "[";
                std::string sep(", ");
                for (auto const &x : set.s) {
                    output_stream << x;
                    if (i++ != last_index) {
                        output_stream << sep;
                    }
                }
                output_stream << "]";
                return output_stream;
            }

        private:
            std::set<T> s;
        };

        template<class T>
        class List {
        public:
            /** Adds element at the end */
            void add(T t) noexcept {
                v.push_back(t);
            }

            void add(long index, T t) noexcept {
                v.insert(v.begin() + index, t);
            }

            void addAll(List<T> &other) noexcept {
                for (auto const &t : other.v) {
                    add(t);
                }
            }

            void addAll(Set<T> &other) noexcept {
                for (auto const &t : other.getStdSet()) {
                    add(t);
                }
            }

            void clear() noexcept {
                v.clear();
            }

            /** Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not contain the element. */
            long indexOf(T t) const noexcept {
                long i = 0;
                for (auto const &e : v) {
                    if (e == t) {
                        return i;
                    }
                    i++;
                }
                return -1;
            }

            bool contains(T t) noexcept {
                return indexOf(t) > -1;
            }

            T get(unsigned long index) const noexcept(false) {
                if (std::size_t(index) >= v.size()) {
                    throw std::out_of_range("get(long index): index out of range exception");
                }
                return v.at(std::size_t(index));
            }

            List<T> subList(unsigned long start, unsigned long length) const {
                List<T> result;
                if (size() == 0) {
                    return result;
                }
                unsigned long max_offset = size() - 1;
                for (unsigned long i = 0; i < length; i++) {
                    if (start + i <= max_offset) {
                        result.add(get(start + i));
                    }
                }
                return result;
            }

            unsigned long size() const noexcept {
                return v.size();
            }

            bool isEmpty() const noexcept {
                return v.empty();
            }

            /** Replaces the element at the specified position in this list with the specified element (optional operation). */
            T set(long index, T t) noexcept(false) {
                if (index >= v.size()) {
                    throw std::out_of_range("set(long index, T t): index out of range exception");
                }
                T old_value = v.at(index);
                v.at(index) = t;
                return old_value;
            }

            T removeAt(unsigned long index) noexcept(false) {
                if (index >= v.size()) {
                    throw std::out_of_range("set(long index): index out of range exception");
                }

                T old_value = v.at(index);
                v.erase(v.begin() + index);
                return old_value;
            }

            bool remove(T t) noexcept {
                long index = indexOf(t);
                if (index == -1) {
                    return false;
                }
                removeAt(index);
                return true;
            }

            bool removeAll(List<T> &other) {
                unsigned long original_size = v.size();
                for (auto const &t : other.v) {
                    remove(t);
                }
                return original_size > v.size();
            }

            std::vector<T> getStdVectorCopy() const noexcept {
                return v;
            }

            std::vector<T> &getStdVector() noexcept {
                return v;
            }

            friend std::ostream &operator<<(std::ostream &output_stream, List<T> list) {
                output_stream << "[";
                auto it = list.v.begin();
                for (auto const &item : list.v) {
                    output_stream << item;
                    it++;
                    if (it != list.v.end()) {
                        output_stream << ", ";
                    }
                }
                output_stream << "]";
                return output_stream;
            }

        private:
            std::vector<T> v;
        };

        template<class K, class V>
        class Map {
        public:
            explicit Map(V def_empty_val) : default_empty_value(def_empty_val) {
            }

            void clear() noexcept {
                m.clear();
            }

            bool isEmpty() const noexcept {
                return m.empty();
            }

            Set<std::pair<K, V>> entrySet() const noexcept {
                Set<std::pair<K, V>> result;
                for (auto const &entry_pair : m) {
                    result.add(entry_pair);
                }
                return result;
            }

            Set<K> keySet() const noexcept {
                Set<K> result;
                for (std::pair<K, V> const &entry_pair : m) {
                    result.add(entry_pair.first);
                }
                return result;
            }

            List<K> keyList() const noexcept {
                List<K> result;
                for (std::pair<K, V> const &entry_pair : m) {
                    result.add(entry_pair.first);
                }
                return result;
            }

            V get(K key) const noexcept {
                auto it = m.find(key);
                if (it == m.end()) {
                    return default_empty_value;
                }
                return it->second;
            }

            bool containsKey(K key) const noexcept {
                if (m.empty()) {
                    return false;
                }
                bool r = m.find(key) != m.end();
                return r;
            }

            V remove(K key) {
                V result = default_empty_value;
                if (containsKey(key)) {
                    result = get(key);
                }
                m.erase(key);
                return result;
            }

            void put(K key, V value) noexcept {
                if (containsKey(key)) {
                    m.erase(key);
                }
                m.insert(std::make_pair(key, value));
            }

            unsigned long size() const noexcept {
                return m.size();
            }

            void putAll(Map<K, V> other) noexcept {
                auto entry_set = other.entrySet().getStdSet();
                for (auto const &entry : entry_set) {
                    put(entry.first, entry.second);
                }
            }

            std::map<K, V> getStdMapCopy() const noexcept {
                return m;
            }

            std::map<K, V> &getStdMap() noexcept {
                return m;
            }

            bool operator==(const Map<K, V> &other) const {
                return this == &other;
            }

            friend std::ostream &operator<<(std::ostream &output_stream, Map<K, V> map) {
                output_stream << "{";
                auto it = map.m.begin();
                for (auto const &pair : map.m) {
                    output_stream << pair.first << ":" << pair.second;
                    it++;
                    if (it != map.m.end()) {
                        output_stream << ", ";
                    }
                }
                output_stream << "}";
                return output_stream;
            }

        private:
            std::map<K, V> m;
            V default_empty_value;
        };

        /////////////////////////////////////////////////

        inline std::size_t maxRand(std::size_t maxInclusive) noexcept {
            if (maxInclusive == 0) {
                return 0;
            }
            return static_cast<std::size_t>(std::rand()) / ((RAND_MAX + 1u) / maxInclusive);
        }

        inline List<std::string> split(std::string &str, char delim) {
            std::stringstream ss(str);
            List<std::string> results;
            std::string item;
            while (getline(ss, item, delim)) {
                results.add(item);
            }
            return results;
        }

        inline List<std::string> split(const std::string &str) {
            std::string str_copy = str;
            return split(str_copy, ' ');
        }

        inline bool startsWith(std::string &source, std::string &target) {
            return source.compare(0, target.length(), target) == 0;
        }

        inline std::int64_t timeInMillis() noexcept {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        }

        /** Taken from https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++ */
        inline std::size_t levenshteinDistance(std::string source, std::string target) {
            // To change the type this function manipulates and returns, change
            // the return type and the types of the two variables below.
            std::size_t s1len = source.size();
            std::size_t s2len = target.size();

            std::size_t column_start = 1;

            // TODO: this should be a unique_ptr
            auto column = new decltype(s1len)[s1len + 1];
            std::iota(column + column_start - 1, column + s1len + 1, column_start - 1);

            for (auto x = column_start; x <= s2len; x++) {
                column[0] = x;
                auto last_diagonal = x - column_start;
                for (auto y = column_start; y <= s1len; y++) {
                    auto old_diagonal = column[y];
                    column[y] = std::min({
                                         column[y] + 1,
                                         column[y - 1] + 1,
                                         last_diagonal + (source[y - 1] == target[x - 1] ? 0 : 1)
                                         });
                    last_diagonal = old_diagonal;
                }
            }
            auto result = column[s1len];
            delete[] column;
            return result;
        }
    }
}

#endif //YUCA_UTILS_HPP
