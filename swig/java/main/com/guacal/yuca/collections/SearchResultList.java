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

import com.guacal.yuca.SearchResult;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

@SuppressWarnings("unused")
public class SearchResultList implements List<SearchResult> {
    private final com.guacal.yuca.swig.SearchResultList swig;

    public SearchResultList(com.guacal.yuca.swig.SearchResultList searchResultList) {
        swig = searchResultList;
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
        if (!(o instanceof SearchResult)) {
            return false;
        }
        return swig.contains(((SearchResult) o).swig());
    }

    @Override
    public Iterator<SearchResult> iterator() {
        return new YucaListIterator<>(this);
    }

    @Override
    public Object[] toArray() {
        SearchResult[] r = new SearchResult[size()];
        for (int i=0; i < r.length; i++) {
            r[i] = new SearchResult(swig.get(i));
        }
        return r;
    }

    @Override
    public <T> T[] toArray(T[] a) {
        return (T[]) toArray();
    }

    @Override
    public boolean add(SearchResult sr) {
        swig.add(sr.swig());
        return true;
    }

    @Override
    public boolean remove(Object o) {
        return swig.remove(((SearchResult) o).swig());
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        SearchResultList other = (SearchResultList) c;
        return swig.containsAll(other.swig);
    }

    @Override
    public boolean addAll(Collection<? extends SearchResult> c) {
        SearchResultList other = (SearchResultList) c;
        swig.addAll(other.swig);
        return true;
    }

    @Override
    public boolean addAll(int index, Collection<? extends SearchResult> c) {
        return false;
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        SearchResultList other = (SearchResultList) c;
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
    public SearchResult get(int index) {
        return new SearchResult(swig.get(index));
    }

    @Override
    public SearchResult set(int index, SearchResult element) {
        return new SearchResult(swig.set(index, element.swig()));
    }

    @Override
    public void add(int index, SearchResult element) {
        swig.add(index, element.swig());
    }

    @Override
    public SearchResult remove(int index) {
        return new SearchResult(swig.removeAt(index));
    }

    @Override
    public int indexOf(Object o) {
        return swig.indexOf(((SearchResult) o).swig());
    }

    @Override
    public int lastIndexOf(Object o) {
        return 0;
    }

    @Override
    public ListIterator<SearchResult> listIterator() {
        return null;
    }

    @Override
    public ListIterator<SearchResult> listIterator(int index) {
        return null;
    }

    @Override
    public List<SearchResult> subList(int fromIndex, int toIndex) {
        com.guacal.yuca.swig.SearchResultList searchResultList = swig.subList(fromIndex, toIndex);
        return new SearchResultList(searchResultList);
    }
}
