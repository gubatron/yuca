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
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>
#include <random>

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

            void addAll(const Set<T> &other_set) {
                for (auto const &x : other_set.s) {
                    add(x);
                }
            }

            bool contains(T const &something) const {
                return s.find(something) != s.end();
            }

            bool containsAll(const Set<T> &other) const {
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

            void removeAll(const Set<T> &other_set) {
                if (this == &other_set) {
                    clear();
                    return;
                }
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
            std::set<T> getStdSetCopy() const {
                return s;
            }

            std::set<T> &getStdSet() & noexcept {
                return s;
            }

            const std::set<T> &getStdSet() const & noexcept {
                return s;
            }

            std::set<T> getStdSet() && noexcept {
                return std::move(s);
            }

            std::set<T> getStdSet() const && {
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
            void add(const T &t) {
                v.push_back(t);
            }

            void add(long index, const T &t) {
                if (index < 0 || static_cast<unsigned long>(index) > v.size()) {
                    throw std::out_of_range("add(long index, T t): index out of range");
                }
                v.insert(v.begin() + index, t);
            }

            void addAll(const List<T> &other) {
                if (this == &other) {
                    const List<T> copy(other);
                    addAll(copy);
                    return;
                }
                v.insert(v.end(), other.v.begin(), other.v.end());
            }

            void addAll(const Set<T> &other) {
                for (auto const &t : other.getStdSet()) {
                    add(t);
                }
            }

            void clear() noexcept {
                v.clear();
            }

            /** Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not contain the element. */
            long indexOf(const T& t) const {
                long i = 0;
                for (auto const &e : v) {
                    if (e == t) {
                        return i;
                    }
                    i++;
                }
                return -1;
            }

            bool contains(const T& t) const {
                return indexOf(t) > -1;
            }

            bool containsAll(const List<T> &other) const {
                for (const T &x : other.v) {
                    if (!contains(x)) {
                        return false;
                    }
                }
                return true;
            }

            T get(unsigned long index) const noexcept(false) {
                if (std::size_t(index) >= v.size()) {
                    throw std::out_of_range("get(long index): index out of range exception");
                }
                return v.at(std::size_t(index));
            }

            List<T> subList(unsigned long start, unsigned long length) const {
                List<T> result;
                if (start >= size()) {
                    return result;
                }
                const unsigned long count = std::min(length, size() - start);
                result.v.assign(v.begin() + static_cast<typename std::vector<T>::difference_type>(start),
                                v.begin() + static_cast<typename std::vector<T>::difference_type>(start + count));
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
                if (index < 0 || static_cast<unsigned long>(index) >= v.size()) {
                    throw std::out_of_range("set(long index, T t): index out of range exception");
                }
                T old_value = v.at(static_cast<unsigned long>(index));
                v.at(static_cast<unsigned long>(index)) = t;
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

            bool remove(T t) {
                long index = indexOf(t);
                if (index == -1) {
                    return false;
                }
                removeAt(index);
                return true;
            }

            bool removeAll(const List<T> &other) {
                unsigned long original_size = v.size();
                if (this == &other) {
                    clear();
                    return original_size != 0;
                }
                for (auto const &t : other.v) {
                    remove(t);
                }
                return original_size > v.size();
            }

            std::vector<T> getStdVectorCopy() const {
                return v;
            }

            std::vector<T> &getStdVector() & noexcept {
                return v;
            }

            const std::vector<T> &getStdVector() const & noexcept {
                return v;
            }

            std::vector<T> getStdVector() && noexcept {
                return std::move(v);
            }

            std::vector<T> getStdVector() const && {
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

            Set<std::pair<K, V>> entrySet() const {
                Set<std::pair<K, V>> result;
                for (auto const &entry_pair : m) {
                    result.add(entry_pair);
                }
                return result;
            }

            Set<K> keySet() const {
                Set<K> result;
                for (auto const &entry_pair : m) {
                    result.add(entry_pair.first);
                }
                return result;
            }

            List<K> keyList() const {
                List<K> result;
                for (auto const &entry_pair : m) {
                    result.add(entry_pair.first);
                }
                return result;
            }

            List<V> values() const {
                List<V> result;
                for (auto const &entry_pair : m) {
                    result.add(entry_pair.second);
                }
                return result;
            }	  

            V get(const K &key) const {
                auto it = m.find(key);
                if (it == m.end()) {
                    return default_empty_value;
                }
                return it->second;
            }

            bool containsKey(const K &key) const {
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

            void put(K key, V value) {
                if (containsKey(key)) {
                    m.erase(key);
                }
                m.insert(std::make_pair(key, value));
            }

            unsigned long size() const noexcept {
                return m.size();
            }

            void putAll(const Map<K, V> &other) {
                auto entry_set = other.entrySet().getStdSet();
                for (auto const &entry : entry_set) {
                    put(entry.first, entry.second);
                }
            }

            std::map<K, V> getStdMapCopy() const {
                return m;
            }

            std::map<K, V> &getStdMap() & noexcept {
                return m;
            }

            const std::map<K, V> &getStdMap() const & noexcept {
                return m;
            }

            std::map<K, V> getStdMap() && noexcept {
                return std::move(m);
            }

            std::map<K, V> getStdMap() const && {
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

        /** Uniform sample in [0, maxInclusive] using the legacy process RNG; not cryptographic. */
        inline std::size_t maxRand(std::size_t maxInclusive) {
            if (maxInclusive == 0) {
                return 0;
            }
            struct RandSource {
                typedef unsigned int result_type;
                static constexpr result_type min() { return 0; }
                static constexpr result_type max() { return RAND_MAX; }
                // Legacy, non-cryptographic RNG; preserve srand() compatibility.
                // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.rand)
                result_type operator()() { return static_cast<result_type>(std::rand()); }
            } source;
            std::uniform_int_distribution<std::size_t> distribution(0, maxInclusive);
            return distribution(source);
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
            std::istringstream input(str);
            List<std::string> results;
            std::string word;
            while (input >> word) {
                results.add(word);
            }
            return results;
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
