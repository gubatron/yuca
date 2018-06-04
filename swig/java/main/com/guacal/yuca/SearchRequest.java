/**
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

import java.util.List;
import java.util.Map;

public final class SearchRequest {
    private final com.guacal.yuca.swig.SearchRequest swig;

    SearchRequest(com.guacal.yuca.swig.SearchRequest swig) {
        this.swig = swig;
    }

    SearchRequest(String queryString, String implicitTag) {
        this(new com.guacal.yuca.swig.SearchRequest(queryString, implicitTag));
    }

    public com.guacal.yuca.swig.SearchRequest swig() {
        return swig;

//        swig.getId();
//        swig.getKeywords(String tag);
//        swig.getQuery() -> String
//                swig.getTag_keywords_map();
//        swig.getTags() -> List<String>
//                swig.getTotal_keywords() -> int
//                swig.op_eq()
//        //swig.setTag_keywords_map()
//        com.guacal.yuca.swig.SearchRequest sr = new com.guacal.yuca.swig.SearchRequest(String query_str, String implicit_tag)

    }

    public void delete() {
        swig.delete();
    }

    public int getId() {
        return swig.getId();
    }

    public String getQuery() {
        return swig.getQuery();
    }

    public List<String> getTags() {
        return new StringList(swig.getTags());
    }

    public int getTotalKeywords() {
        return swig.getTotal_keywords();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof SearchRequest)) {
            return false;
        }
        return swig.op_eq(((SearchRequest) obj).swig);
    }

    public Map<String, List<String>> getTagKeywordsMap(){
        //swig.getTag_keywords_map()
        return null;
    }
}
