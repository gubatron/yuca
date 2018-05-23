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
        class List;

        template<class K, class V>
        class Map;

        std::size_t maxRand(std::size_t maxInclusive);
        yuca::utils::List<std::string> split(std::string &str, char delim);
        yuca::utils::List<std::string> split(const std::string &str);
        bool startsWith(std::string &source, std::string &target);
        std::int64_t timeInMillis() noexcept;
        std::size_t levenshteinDistance(std::string source, std::string target);
    }

}

class yuca::Key;
class yuca::StringKey;

typedef std::shared_ptr<Key> yuca::SPKey;
typedef std::shared_ptr<StringKey> yuca::SPStringKey;
typedef yuca::utils::Set<Key> yuca::KeySet;
typedef yuca::utils::Set<SPKey> yuca::SPKeySet;
typedef yuca::utils::List<SPKey> yuca::SPKeyList;

class yuca::Document;
typedef std::shared_ptr<Document> yuca::SPDocument;
typedef yuca::utils::Set<SPDocument> yuca::SPDocumentSet;
typedef yuca::utils::List<SPDocument> yuca::SPDocumentList;

%template(Set_Key) yuca::utils::Set<yuca::Key>;
%template(Set_SPDocument) yuca::utils::Set<yuca::SPDocument>;

class yuca::Indexer;

%ignore operator();
%rename(op_eq) operator==;
%rename(op_ne) operator!=;
%rename(op_lt) operator<;
%rename(op_gt) operator>;
%rename(op_lte) operator<=;
%rename(op_gte) operator>=;
%rename(op_bool) operator bool;
