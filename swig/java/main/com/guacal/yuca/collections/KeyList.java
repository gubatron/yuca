/*
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

package com.guacal.yuca.collections;

import com.guacal.yuca.Key;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class KeyList implements List<Key> {
    private final com.guacal.yuca.swig.KeyList swig;

    public KeyList(com.guacal.yuca.swig.KeyList keyList) {
        swig = keyList;
    }

    @Override
    public int size() {
        return (int) swig.size();
    }

    @Override
    public boolean isEmpty() {
        return swig.isEmpty();
    }

    @Override
    public boolean contains(Object o) {
        if (!(o instanceof Key)) {
            return false;
        }
        return swig.contains(((Key) o).swig());
    }

    @Override
    public Iterator<Key> iterator() {
        return new YucaListIterator<>(this);
    }

    @Override
    public Object[] toArray() {
        Key[] r = new Key[size()];
        for (int i=0; i < r.length; i++) {
            r[i] = new Key(swig.get(i));
        }
        return r;
    }

    @Override
    public <T> T[] toArray(T[] a) {
        return (T[]) toArray();
    }

    @Override
    public boolean add(Key key) {
        swig.add(key.swig());
        return true;
    }

    @Override
    public boolean remove(Object o) {
        return swig.remove(((Key) o).swig());
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        KeyList other = (KeyList) c;
        return swig.containsAll(other.swig);
    }

    @Override
    public boolean addAll(Collection<? extends Key> c) {
        KeyList other = (KeyList) c;
        swig.addAll(other.swig);
        return true;
    }

    @Override
    public boolean addAll(int index, Collection<? extends Key> c) {
        return false;
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        KeyList other = (KeyList) c;
        return swig.removeAll(other.swig);
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        return false;
    }

    @Override
    public void clear() {
        swig.clear();
    }

    @Override
    public Key get(int index) {
        return new Key(swig.get(index));
    }

    @Override
    public Key set(int index, Key element) {
        return new Key(swig.set(index, element.swig()));
    }

    @Override
    public void add(int index, Key element) {
        swig.add(index, element.swig());
    }

    @Override
    public Key remove(int index) {
        return new Key(swig.removeAt(index));
    }

    @Override
    public int indexOf(Object o) {
        return swig.indexOf(((Key) o).swig());
    }

    @Override
    public int lastIndexOf(Object o) {
        return 0;
    }

    @Override
    public ListIterator<Key> listIterator() {
        return null;
    }

    @Override
    public ListIterator<Key> listIterator(int index) {
        return null;
    }

    @Override
    public List<Key> subList(int fromIndex, int toIndex) {
        com.guacal.yuca.swig.KeyList keyList = swig.subList(fromIndex, toIndex);
        return new KeyList(keyList);
    }
}
