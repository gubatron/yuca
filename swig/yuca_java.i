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
        class Set;

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

typedef std::shared_ptr<Key> SPKey;
typedef std::shared_ptr<StringKey> SPStringKey;
typedef yuca::utils::Set<Key> KeySet;
typedef yuca::utils::Set<SPKey> SPKeySet;
typedef yuca::utils::List<SPKey> SPKeyList;

class yuca::Document;
typedef std::shared_ptr<Document> SPDocument;
typedef yuca::utils::Set<SPDocument> SPDocumentSet;
typedef yuca::utils::List<SPDocument> SPDocumentList;

class yuca::Indexer;

%ignore operator();
%rename(op_eq) operator==;
%rename(op_ne) operator!=;
%rename(op_lt) operator<;
%rename(op_gt) operator>;
%rename(op_lte) operator<=;
%rename(op_gte) operator>=;
%rename(op_bool) operator bool;
