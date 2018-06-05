/*
 * MIT License
 * <p>
 * Copyright (c) 2016-2018 Angel Leon, Alden Torres
 * <p>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * <p>
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * <p>
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package com.guacal.yuca;

import java.util.List;

public class Indexer {
    private com.guacal.yuca.swig.Indexer swig;

    public Indexer() {
        swig = new com.guacal.yuca.swig.Indexer(); // implicit group will be :group
    }

    public Indexer(final String implicitGroup) {
        swig = new com.guacal.yuca.swig.Indexer(implicitGroup);
    }

    public void indexDocument(final Document document) {
        swig.indexDocument(document.swig());
    }

    public Document getDocument(final int docId) {
        return new Document(swig.getDocument(docId));
    }

    public Document getDocument(final String docId) {
        return new Document(swig.getDocument(docId));
    }

    boolean removeDocument(final int docId) {
        return swig.removeDocument(docId);
    }

    boolean removeDocument(final String docId) {
        return swig.removeDocument(docId);
    }

    boolean removeDocument(final Document doc) {
        return swig.removeDocument(doc.swig());
    }

    void clear() {
        swig.clear();
    }

    List<SearchResult> search(final String query,
                              final String optMainDocPropertyForQueryComparison,
                              final int optMaxSearchResults) {
        return new com.guacal.yuca.collections.SearchResultList(swig.search(query, optMainDocPropertyForQueryComparison, optMaxSearchResults));
    }

    List<SearchResult> search(final String query,
                              final String optMainDocPropertyForQueryComparison) {
        return new com.guacal.yuca.collections.SearchResultList(swig.search(query, optMainDocPropertyForQueryComparison));
    }

    List<SearchResult> search(final String query) {
        return new com.guacal.yuca.collections.SearchResultList(swig.search(query));
    }
}
