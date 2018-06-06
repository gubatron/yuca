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

package com.guacal.yuca;

public class StringKey extends Key {

    private final com.guacal.yuca.swig.StringKey swig;

    public StringKey(com.guacal.yuca.swig.StringKey swigKey) {
        this.swig = swigKey;
    }

    public StringKey(String stringKey, String group) {
        swig = new com.guacal.yuca.swig.StringKey(stringKey, group);
    }

    public com.guacal.yuca.swig.StringKey swig() {
        return swig;
    }

    public int getId() {
        return swig.getId();
    }

    public String getString() {
        return swig.getString();
    }

    public String getGroup() { return swig.getGroup(); }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof StringKey)) {
            return false;
        }
        return swig.op_eq(((StringKey) obj).swig);
    }
}
