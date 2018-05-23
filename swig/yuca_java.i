%module (jniclassname="yuca_jni") yuca

%rename(op_eq) operator==;
%rename(op_ne) operator!=;
%rename(op_lt) operator<;
%rename(op_gt) operator>;
%rename(op_lte) operator<=;
%rename(op_gte) operator>=;

%include <std_string.i>
%include <stdint.i>
%include <std_shared_ptr.i>

%{
#include "yuca/yuca.hpp"
#include "yuca/utils.hpp"
#include "yuca/key.hpp"
#include "yuca/document.hpp"
#include "yuca/indexer.hpp"
%}

namespace yuca {
    std::string version();

    namespace utils {
        const std::string VERSION;
        const std::string LAST_UPDATED;

        template <class T>
        class Set {
        public:
            void add(T const &e);
            void addAll(Set<T> &other_set) noexcept;
            bool contains(T const &something) const noexcept;
            bool containsAll(Set<T> &other) const noexcept;
            bool isEmpty() const noexcept;
            bool remove(T something);
            void removeAll(Set<T> &other_set);
            void clear() noexcept;
  	    unsigned long size() const noexcept;
	};

        template <class T>
        class List {
        public:
            void add(T t) noexcept;
            void add(long index, T t) noexcept;
            void addAll(List<T> &other) noexcept;
            void addAll(Set<T> &other) noexcept;
            void clear() noexcept;
            long indexOf(T t) const noexcept; 
            bool contains(T t) noexcept;
            T get(unsigned long index) const noexcept(false);
            List<T> subList(unsigned long start, unsigned long length) const;
            unsigned long size() const noexcept;
            bool isEmpty() const noexcept;
            T set(long index, T t) noexcept(false);
            T removeAt(unsigned long index) noexcept(false);
            bool remove(T t) noexcept;
            bool removeAll(List<T> &other);
	};

        template<class K, class V>
        class Map {
        public:
            explicit Map(V def_empty_val) : default_empty_value(def_empty_val);
            void clear() noexcept;
            bool isEmpty() const noexcept;
            Set<std::pair<K, V>> entrySet() const noexcept;
            Set<K> keySet() const noexcept;
            List<K> keyList() const noexcept;
            V get(K key) const noexcept;
            bool containsKey(K key) const noexcept;
            V remove(K key);
            void put(K key, V value) noexcept;
            unsigned long size() const noexcept;
            void putAll(Map<K, V> other) noexcept;
        };

        std::size_t maxRand(std::size_t maxInclusive);
        yuca::utils::List<std::string> split(std::string &str, char delim);
        yuca::utils::List<std::string> split(const std::string &str);
        bool startsWith(std::string &source, std::string &target);
        std::int64_t timeInMillis() noexcept;
        std::size_t levenshteinDistance(std::string source, std::string target);
    }

    class Key {
    public:
        Key() = default;
        explicit Key(long my_id, std::string my_tag) : id(my_id), tag(std::move(my_tag));
        std::string getTag() const;
        long getId() const;

        %extend {
          bool op_lt(const Key &right_side) const {
            return *$self < right_side;
          }

          bool op_lte(const Key &right_side) const {
            return *$self <= right_side;
          }

          bool op_gt(const Key &right_side) const {
            return *$self > right_side;
          }

          bool op_gte(const Key &right_side) const {
            return *$self >= right_side;
          }

          bool op_gt(const Key &right_side) const {
            return *$self >= right_side;
          }
        }
    };
}


class yuca::StringKey;

//typedef std::shared_ptr<Key> yuca::SPKey;
%template(SPKey) std::shared_ptr<yuca::Key>;
//typedef std::shared_ptr<StringKey> yuca::SPStringKey;
%template(SPStringKey) std::shared_ptr<yuca::StringKey>;
//typedef yuca::utils::Set<Key> yuca::KeySet;
%template(KeySet) yuca::utils::Set<yuca::Key>;
//typedef yuca::utils::Set<SPKey> yuca::SPKeySet;
%template(SPKeySet) yuca::utils::Set<yuca::SPKey>;
//typedef yuca::utils::List<SPKey> yuca::SPKeyList;
%template(SPKeyList) yuca::utils::List<yuca::SPKey>;

class yuca::Document;
//typedef std::shared_ptr<Document> yuca::SPDocument;
%template(SPDocument) std::shared_ptr<yuca::Document>;
//typedef yuca::utils::Set<SPDocument> yuca::SPDocumentSet;
%template(SPDocumentSet) yuca::utils::Set<yuca::SPDocument>;
//typedef yuca::utils::List<SPDocument> yuca::SPDocumentList;
%template(SPDocumentList) yuca::utils::List<yuca::SPDocument>;

class yuca::Indexer;

%ignore operator();
%rename(op_eq) operator==;
%rename(op_ne) operator!=;
%rename(op_lt) operator<;
%rename(op_gt) operator>;
%rename(op_lte) operator<=;
%rename(op_gte) operator>=;
%rename(op_bool) operator bool;
