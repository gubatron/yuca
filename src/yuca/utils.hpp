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

namespace yuca {
	namespace utils {
		template<class T>
		class Set {
		public:
			void add(T e) {
				s.insert(e);
			}

			void addAll(Set<T> other_set) noexcept {
				for (auto const &x : other_set.s) {
					add(x);
				}
			}

			bool contains(T something) const noexcept {
				return s.find(something) != s.end();
			}

			bool containsAll(Set<T> other) const noexcept {
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

			void removeAll(Set<T> other_set) {
				for (auto const &x : other_set.s) {
					remove(x);
				}
			}

			void clear() {
				s.clear();
			}

			/**
			 * @return a copied std::set<T> instance of the containing std::set<T> s
			 */
			std::set<T> getStdSet() const {
				return s;
			}

			unsigned long size() const {
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

		template <class T>
		class List {
		public:
			/** Adds element at the end */
			void add(T t) noexcept {
				v.push_back(t);
			}

			void add(long index, T t) noexcept {
				v.insert(v.begin() + index, t);
			}

			void addAll(List<T> other) noexcept {
				for (auto const& t : other.v) {
					add(t);
				}
			}

			void addAll(Set<T> other) noexcept {
				for (auto const& t : other.getStdSet()) {
					add(t);
				}
			}

			void clear() noexcept {
				v.clear();
			}

			/** Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not contain the element. */
			long indexOf(T t) const noexcept {
				long i=0;
				for (auto const& e : v) {
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

			T get(long index) const noexcept(false) {
				if (index >= v.size()) {
					throw("get(long index): index out of bounds exception");
				}
				return v.at(index);
			}

			List<T> subList(long start, long length) {
				List<T> result;
				for (long i=0; i < length; i++) {
					result.add(get(start + i));
				}
				return result;
			}

			long size() const noexcept {
				return v.size();
			}

			bool isEmpty() const noexcept {
				return v.size() == 0;
			}

			/** Replaces the element at the specified position in this list with the specified element (optional operation). */
			T set(long index, T t) noexcept(false) {
				if (index >= v.size()) {
					throw("set(long index, T t): index out of bounds exception");
				}
				T old_value = v.at(index);
				v.at(index) = t;
				return old_value;
			}

			T removeAt(long index) noexcept(false) {
				if (index >= v.size()) {
					throw ("set(long index): index out of bounds exception");
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

			bool removeAll(List<T> other) {
				long original_size = v.size();
				for (auto const &t : other.v) {
					remove(t);
				}
				return original_size > v.size();
			}

			std::vector<T> getStdVector() {
				return v;
			}

		private:
			std::vector<T> v;
			//const T default_empty_value;
		};

		template<class K, class V>
		class Map {
		public:
			explicit Map(V def_empty_val) : default_empty_value(def_empty_val) {
			}

			void clear() {
				m.clear();
			}

			bool isEmpty() const {
				return m.empty();
			}

			Set<std::pair<K, V>> entrySet() const {
				Set<std::pair<K, V>> result;
				for (auto const &entry_pair : m) {
					result.add(entry_pair);
				}
				return result;
			};

			Set<K> keySet() const {
				Set<K> result;
				auto it = m.begin();
				for (std::pair<K, V> const &entry_pair : m) {
					result.add(entry_pair.first);
				}
				return result;
			}

			List<K> keyList() const {
				List<K> result;
				auto it = m.begin();
				for (std::pair<K, V> const &entry_pair : m) {
					result.add(entry_pair.first);
				}
				return result;
			}

			V get(K key) const {
				auto it = m.find(key);
				if (it == m.end()) {
					return default_empty_value;
				}
				return it->second;
			}

			bool containsKey(K key) const {
				if (m.empty()) {
					return false;
				}
				return m.find(key) != m.end();
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

			unsigned long size() const {
				return m.size();
			}

			void putAll(Map<K, V> other) {
				auto entry_set = other.entrySet().getStdSet();
				for (auto const &entry : entry_set) {
					put(entry.first, entry.second);
				}
			}

			std::map<K, V> getStdMap() const {
				return m;
			}

			bool operator==(const Map<K,V> &other) const {
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
			const V default_empty_value;
		};

		/////////////////////////////////////////////////

		inline int maxRand(int maxInclusive) {
			return std::rand()/((RAND_MAX + 1u)/maxInclusive);
		};

		inline List<std::string> split(std::string &str, char delim) {
			std::stringstream ss(str);
			List<std::string> results;
			std::string item;
			while (getline(ss, item, delim)) {
				results.add(item);
			}
			return results;
		}

		inline List<std::string> split(std::string &str) {
			return split(str, ' ');
		}

		inline bool startsWith(std::string &source, std::string &target) {
			return source.compare(0, target.length(), target) == 0;
		}
	};
}

#endif //YUCA_UTILS_HPP
