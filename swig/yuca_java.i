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

          bool op_eq(const Key &right_side) const {
            return *$self == right_side;
          }
        }
    };

    class StringKey : public Key {
    public:
        explicit StringKey(const std::string &string_key, const std::string &my_tag) :
          Key(static_cast<long>(std::hash<std::string>{}(my_tag + string_key)), my_tag), str_key(string_key);

        std::string getString() const;
    };

    enum PropertyType {
        BOOL,
        BYTE,
        INT,
        LONG,
        STRING
    };

    class Document {
    public:
        explicit Document(long doc_id) :
        id(doc_id),
        tag_2_keyset_map(yuca::SPKeySet()),
        bool_properties(false),
        byte_properties(0),
        int_properties(-1),
        long_properties(-1l),
        string_properties("");
        explicit Document(const std::string &str_based_id) : Document(static_cast<long>(std::hash<std::string>{}(str_based_id)));
        long getId() const;
        void addKey(Key const &key);
        void addKey(std::shared_ptr<Key> key);
        bool hasKeys(std::string const &tag) const;
        std::set<std::string> getTags() const;
        yuca::SPKeySet getTagSPKeys(std::string const &tag) const;
        yuca::KeySet getTagKeys(std::string const &tag) const;
        void removeTag(std::string const &tag);
        void removeKey(std::string const &tag, std::shared_ptr<Key> key);
        void boolProperty(std::string const &key, bool value);
        bool boolProperty(std::string const &key) const;
        void removeBoolProperty(std::string const &key);
        void byteProperty(std::string const &key, char value);
        char byteProperty(std::string const &key) const;
        void removeByteProperty(std::string const &key);
        void intProperty(std::string const &key, int value);
        int intProperty(std::string const &key) const;
        void removeIntProperty(std::string const &key);
        void longProperty(std::string const &key, long value);
        long longProperty(std::string const &key) const;
        void removeLongProperty(std::string const &key);
        void stringProperty(std::string const &key, std::string const &value);
        std::string stringProperty(std::string const &key) const;
        void removeStringProperty(std::string const &key);
        yuca::utils::List<std::string> propertyKeys(PropertyType type) const;
        static const Document NULL_DOCUMENT;

        %extend {
          bool op_lt(const Document &right_side) const {
            return *$self < right_side;
          }

          bool op_eq(const Document &right_side) const {
            return *$self == right_side;
          }
        }
    };
}

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
