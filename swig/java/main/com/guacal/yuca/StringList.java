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
import java.util.List;
import java.util.ListIterator;

public class StringList implements List<String> {
    private final com.guacal.yuca.swig.StringList wrappedList;
    StringList(com.guacal.yuca.swig.StringList list) {
        wrappedList = list;
    }

    @Override
    public int size() {
        return (int) wrappedList.size();
    }

    @Override
    public boolean isEmpty() {
        return wrappedList.isEmpty();
    }

    @Override
    public boolean contains(Object o) {
        String other = (String) o;
        return wrappedList.contains(other);
    }

    @Override
    public Iterator<String> iterator() {
        return null;
    }

    @Override
    public Object[] toArray() {
        String[] r = new String[(int) wrappedList.size()];
        for (int i=0; i < r.length; i++) {
            r[i] = wrappedList.get(i);
        }
        return r;
    }

    @Override
    public <T> T[] toArray(T[] a) {
        return (T[]) toArray();
    }

    @Override
    public boolean add(String s) {
        wrappedList.add(s);
        return true;
    }

    @Override
    public boolean remove(Object o) {
        String other = (String) o;
        return wrappedList.remove(other);
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        StringList other = (StringList) c;
        return wrappedList.containsAll(other.wrappedList);
    }

    @Override
    public boolean addAll(Collection<? extends String> c) {
        StringList other = (StringList) c;
        wrappedList.addAll(other.wrappedList);
        return true;
    }

    @Override
    public boolean addAll(int index, Collection<? extends String> c) {
        StringList other = (StringList) c;
        other.wrappedList.addAll(other.wrappedList);
        return true;
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        StringList other = (StringList) c;
        return wrappedList.removeAll(other.wrappedList);
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        return false;
    }

    @Override
    public void clear() {
        wrappedList.clear();
    }

    @Override
    public String get(int index) {
        return wrappedList.get(index);
    }

    @Override
    public String set(int index, String element) {
        return wrappedList.set(index, element);
    }

    @Override
    public void add(int index, String element) {
        wrappedList.add(index, element);
    }

    @Override
    public String remove(int index) {
        return wrappedList.removeAt(index);
    }

    @Override
    public int indexOf(Object o) {
        String other = (String) o;
        return wrappedList.indexOf(other);
    }

    @Override
    public int lastIndexOf(Object o) {
        // TODO
        String other = (String) o;
        return 0;
    }

    @Override
    public ListIterator<String> listIterator() {
        return null;
    }

    @Override
    public ListIterator<String> listIterator(int index) {
        return null;
    }

    @Override
    public List<String> subList(int fromIndex, int toIndex) {
        return new StringList(wrappedList.subList(fromIndex, toIndex));
    }
}
