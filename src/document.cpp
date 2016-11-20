#include "yuca/document.hpp"

namespace yuca {

    DocumentKeys Document::getKeys() const {
        return documentKeys;
    }

    KeySet Document::getKeys(string const &tag) {
        if (!hasKeys(tag)) {
            documentKeys[tag] = KeySet();
            return documentKeys[tag];
        }
        return KeySet();
    }

    void Document::addKey(string const &tag, const Key &key) {
        KeySet keySet = getKeys(tag);
        keySet.insert((Key*) &key);
    }

    bool Document::hasKeys(string const &tag) const {
        return documentKeys.count(tag) > 0;
    }

    void Document::removeKeys(string const &tag) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet keySet = getKeys(tag);
        keySet.clear();
    }

    void Document::removeKey(string const &tag, const Key &key) {
        if (!hasKeys(tag)) {
            return;
        }
        KeySet keySet = getKeys(tag);
        keySet.erase((Key*) &key);
    }
}
