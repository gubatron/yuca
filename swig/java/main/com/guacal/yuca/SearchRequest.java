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

import com.guacal.yuca.collections.StringList;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public final class SearchRequest {
    private final com.guacal.yuca.swig.SearchRequest swig;
    private final int id;
    private final int totalKeywords;
    private final String query;
    private final List<String> groups;
    private final Map<String, List<String>> groupKeywordsMap;


    SearchRequest(com.guacal.yuca.swig.SearchRequest swig) {
        this.swig = swig;
        id = swig.getId();
        totalKeywords = swig.getTotal_keywords();
        query = swig.getQuery();
        groups = new StringList(swig.getTags());

        groupKeywordsMap = new HashMap<>();
        if (groups.size() > 0) {
            for (String group : groups) {
                groupKeywordsMap.put(group, new StringList(swig.getKeywords(group)));
            }
        }
    }

    @SuppressWarnings("unused")
    SearchRequest(String queryString, String implicitTag) {
        this(new com.guacal.yuca.swig.SearchRequest(queryString, implicitTag));
    }

    public com.guacal.yuca.swig.SearchRequest swig() {
        return swig;
    }

    public void delete() {
        swig.delete();
    }

    @SuppressWarnings("unused")
    public int getId() {
        return id;
    }

    @SuppressWarnings("unused")
    public String getQuery() {
        return query;
    }

    @SuppressWarnings("unused")
    public List<String> getGroups() {
        return groups;
    }

    @SuppressWarnings("unused")
    public int getTotalKeywords() {
        return totalKeywords;
    }

    @SuppressWarnings("unused")
    public Map<String, List<String>> getTagKeywordsMap(){
        return groupKeywordsMap;
    }

    @SuppressWarnings("unused")
    public List<String> getKeywords(String group) {
        return groupKeywordsMap.get(group);
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof SearchRequest)) {
            return false;
        }
        return swig.op_eq(((SearchRequest) obj).swig);
    }
}
