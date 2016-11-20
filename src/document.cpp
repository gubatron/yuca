#include "yuca/document.hpp"

namespace yuca {

    DocumentKeys Document::getKeys() const {
        return documentKeys;
    }

    KeySet Document::getKeys(string const &name) const {
        if (documentKeys.count(name) == 0) {
        }
        return KeySet();
    }

    void Document::addKey(string const &name, const Key &key) {

    }

    bool Document::hasKey(string const &name) const {
        return false;
    }

    void Document::removeKeys(string const &name) {

    }

    void Document::removeKey(string const &name, const Key &key) {

    }
}
