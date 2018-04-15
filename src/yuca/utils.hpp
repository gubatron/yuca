//
// Java Collections Inspired wrappers for std:: collections
// Created by gubatron on 3/23/18.
//

#ifndef YUCA_UTILS_HPP
#define YUCA_UTILS_HPP

#include <set>
#include <map>
#include <ostream>
#include <iostream>

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

			//private:
			std::set<T> s;
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
	}
}

#endif //YUCA_UTILS_HPP
