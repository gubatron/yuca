%module (jniclassname="yuca_jni") yuca

%rename(op_eq) operator==;
%rename(op_ne) operator!=;
%rename(op_lt) operator<;
%rename(op_gt) operator>;
%rename(op_lte) operator<=;
%rename(op_gte) operator>=;

%include <std_string.i>
%include <stdint.i>
%include <std_vector.i>
%include <std_map.i>
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
            long indexOf(const T t) const noexcept; 
            bool contains(T t) noexcept;
            bool containsAll(List<T> &other) const noexcept;
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
            explicit Map(V def_empty_val);
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
      explicit Key(long my_id, std::string my_tag);
        std::string getTag() const;
        long getId() const;

        %extend {
          bool op_lt(const Key &right_side) const {
            return $self->getId() < right_side.getId();
          }

          bool op_lte(const Key &right_side) const {
            return $self->getId() <= right_side.getId();
          }

          bool op_gt(const Key &right_side) const {
            return $self->getId() > right_side.getId();
          }

          bool op_gte(const Key &right_side) const {
            return $self->getId() >= right_side.getId();
          }

          bool op_eq(const Key &right_side) const {
            return $self->getId() == right_side.getId();
          }
        }
    };

    class StringKey : public Key {
    public:
        explicit StringKey(const std::string &string_key, const std::string &my_tag);
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
        explicit Document(long doc_id);
        explicit Document(const std::string &str_based_id);
        long getId() const;
        void addKey(Key const &key);
        bool hasKeys(std::string const &tag) const;
%extend {
        yuca::utils::List<std::string> getTagsList() const {
            yuca::utils::List<std::string> r;
            std::set<std::string> tags_set = $self->getTags();
            for (const auto& tag : tags_set) {
                r.add(tag);
            }
            return r;
        }

        yuca::utils::List<yuca::StringKey> getTagKeysList(std::string const &tag) {
            yuca::utils::List<yuca::StringKey> r;
            for(const auto& k : $self->getTagKeys(tag).getStdSetCopy()) {
              r.add(static_cast<const yuca::StringKey>(k));
            }
            return r;
        }
}
        void removeTag(std::string const &tag);
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
            return $self->getId() < right_side.getId();
          }

          bool op_eq(const Document &right_side) const {
            return $self->getId() == right_side.getId();
          }
        }
    };

    struct SearchRequest {
        SearchRequest(const std::string &query_str, const std::string &implicit_tag);
        yuca::utils::List<std::string> getTags();
        yuca::utils::List<std::string> getKeywords(std::string &tag);
      //yuca::utils::Map<std::string, yuca::utils::List<std::string>> tag_keywords_map;
        %extend {
          bool op_eq(const SearchRequest &right_side) const {
            return $self->query == right_side.query && $self->id == right_side.id && $self->total_keywords == right_side.total_keywords;
          }
        }
        const std::string query;
        const long id;
        long total_keywords;
    };

    struct SearchResult {
        SearchResult(std::shared_ptr<yuca::SearchRequest> searchRequest, yuca::SPDocument docSp);
        %extend {
          bool op_eq(const SearchResult &right_side) const {
            return $self->id == right_side.id;
          }
        }
        double score; //[0.0 - 1.0]
        long id;
        std::shared_ptr<SearchRequest> search_request_sp;
        yuca::SPDocument document_sp;
    };

    class Indexer {
    public:
        Indexer(const std::string &an_implicit_tag);
        Indexer();
        void indexDocument(yuca::Document doc);
        yuca::Document getDocument(long doc_id) const noexcept;
        yuca::Document getDocument(std::string const &doc_id) const noexcept;
        bool removeDocument(long doc_id);
        bool removeDocument(std::string const &doc_id);
        void removeDocument(yuca::Document doc);
        void indexDocument(yuca::SPDocument doc); // maybe remove, could be unusable in java
        void removeDocument(yuca::SPDocument doc); // maybe remove, could be unusable in java
        void clear();
        yuca::utils::List<SearchResult> search(const std::string &query,
                                               const std::string &opt_main_doc_property_for_query_comparison,
                                               unsigned long opt_max_search_results) const;
        yuca::utils::List<SearchResult> search(const std::string &query);
        yuca::utils::List<SearchResult> search(const std::string &query,
                                               const std::string &opt_main_doc_property_for_query_comparison);
        //yuca::utils::Map<std::string, yuca::SPDocumentSet> findDocuments(yuca::SearchRequest &search_request) const;
        //yuca::SPDocumentSet findDocuments(yuca::SPKey key) const;
        //yuca::SPDocumentSet findDocuments(yuca::SPKeyList keys) const;
    };
}


%template(SPKey) std::shared_ptr<yuca::Key>;
%template(SPStringKey) std::shared_ptr<yuca::StringKey>;

%template(SPKeySet) yuca::utils::Set<yuca::SPKey>;
%template(SPKeyList) yuca::utils::List<yuca::SPKey>;

%template(SPDocument) std::shared_ptr<yuca::Document>;
%template(SPDocumentSet) yuca::utils::Set<yuca::SPDocument>;
%template(SPDocumentList) yuca::utils::List<yuca::SPDocument>;

%template(KeySet) yuca::utils::Set<yuca::Key>;
%template(KeyList) yuca::utils::List<yuca::Key>;

%template(StringKeyList) yuca::utils::List<yuca::StringKey>;

%template(DocumentSet) yuca::utils::Set<yuca::Document>;
%template(DocumentList) yuca::utils::List<yuca::Document>;

%template(SearchResultList) yuca::utils::List<yuca::SearchResult>;

%template(StringList) yuca::utils::List<std::string>;

%ignore operator();
%rename(op_eq) operator==;
%rename(op_ne) operator!=;
%rename(op_lt) operator<;
%rename(op_gt) operator>;
%rename(op_lte) operator<=;
%rename(op_gte) operator>=;
%rename(op_bool) operator bool;
