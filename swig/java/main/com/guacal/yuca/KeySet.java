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

import java.util.Collection;
import java.util.Iterator;
import java.util.Set;

public final class KeySet implements Set<Key> {
    private final com.guacal.yuca.swig.KeySet wrappedKeySet;

    KeySet(com.guacal.yuca.swig.KeySet keySet) {
        wrappedKeySet = keySet;
    }

    @Override
    public int size() {
        return (int) wrappedKeySet.size();
    }

    @Override
    public boolean isEmpty() {
        return wrappedKeySet.isEmpty();
    }

    @Override
    public boolean contains(Object o) {
        if (!(o instanceof Key)) {
            return false;
        }
        Key other = (Key) o;
        return wrappedKeySet.contains(other.swig());
    }

    @Override
    public Iterator<Key> iterator() {
        return null;
    }

    @Override
    public Object[] toArray() {
        return new Object[0];
    }

    @Override
    public <T> T[] toArray(T[] a) {
        return null;
    }

    @Override
    public boolean add(Key key) {
        wrappedKeySet.add(key.swig());
        return true;
    }

    @Override
    public boolean remove(Object o) {
        return wrappedKeySet.remove(((Key) o).swig());
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        KeySet other = (KeySet) c;
        return wrappedKeySet.containsAll(other.wrappedKeySet);
    }

    @Override
    public boolean addAll(Collection<? extends Key> c) {
        KeySet other = (KeySet) c;
        wrappedKeySet.addAll(other.wrappedKeySet);
        return true;
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        return false;
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        KeySet other = (KeySet) c;
        wrappedKeySet.removeAll(other.wrappedKeySet);
        return true;
    }

    @Override
    public void clear() {
        wrappedKeySet.clear();
    }
}
