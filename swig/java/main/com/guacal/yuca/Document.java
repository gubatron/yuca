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

package com.guacal.yuca;

import com.guacal.yuca.swig.KeySet;
import com.guacal.yuca.swig.PropertyType;

import java.util.List;
import java.util.Set;

public class Document {
    private final com.guacal.yuca.swig.Document swig;

    public Document(int docId) {
        swig = new com.guacal.yuca.swig.Document(docId);
    }

    public Document(String strBasedId) {
        swig = new com.guacal.yuca.swig.Document(strBasedId);
    }

    public com.guacal.yuca.swig.Document swig() {
        return swig;
    }

    public int getId() {
        return swig.getId();
    }

    public void addKey(Key key) {
        this.swig.addKey(key.swig());
    }

    public Set<Key> getTagKeys(String tag) {
        KeySet tagKeys = swig.getTagKeys(tag);
        return new com.guacal.yuca.KeySet(tagKeys);
    }

    public boolean boolProperty(String key) {
        return swig.boolProperty(key);
    }

    public void boolProperty(String key, boolean value) {
        swig.boolProperty(key, value);
    }

    public char byteProperty(String key) {
        return swig.byteProperty(key);
    }

    public void byteProperty(String key, char value) {
        swig.byteProperty(key, value);
    }

    public void delete() {
        swig.delete();
    }

    public List<String> getTags() {
        return new StringList(swig.getTagsList());
    }

    public boolean hasKeys(String tag) {
        return swig.hasKeys(tag);
    }

    public int intProperty(String key) {
        return swig.intProperty(key);
    }

    public void intProperty(String key, int value) {
        swig.intProperty(key, value);
    }

    public int longProperty(String key) {
        return swig.longProperty(key);
    }

    public void longProperty(String key, long value) {
        swig.longProperty(key, (int) value);
    }

    @Override
    public boolean equals(Object obj) {
        Document other = (Document) obj;
        return other.swig.op_eq(other.swig);
    }

    @Override
    public int hashCode() {
        return swig.getId();
    }

    public void removeBoolProperty(String key) {
        swig.removeBoolProperty(key);
    }

    public void removeByteProperty(String key) {
        swig.removeByteProperty(key);
    }

    public void removeIntProperty(String key) {
        swig.removeIntProperty(key);
    }

    public void removeLongProperty(String key) {
        swig.removeLongProperty(key);
    }

    public void removeStringProperty(String key) {
        swig.removeStringProperty(key);
    }

    public void removeTag(String tag) {
        swig.removeTag(tag);
    }

    public List<String> propertyList(PropertyType propertyType) {
        return new StringList(swig.propertyKeys(propertyType));
    }
}
