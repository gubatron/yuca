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

import com.guacal.yuca.swig.yuca;

import java.util.List;

public final class Demo {
    static {
        try {
            // Temporary
            System.load("/Users/gubatron/workspace.frostwire/yuca/swig/libyuca_jni_shared.dylib");
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public static void main(String[] args) {
        System.out.println("Yuca Java Wrapper version: " + yuca.version() + "\nLast Updated: " + yuca.getLAST_UPDATED() + "\n\n");

        StringKey fooKey = new StringKey("fooKey",":fooGroup");
        StringKey fooKey2 = new StringKey("fooKey2",":fooGroup");
        StringKey barKey = new StringKey("barKey", ":barGroup");

        System.out.println("<" + barKey.getId() + ":" + barKey.getString() + " group= " + barKey.getGroup() + ">");

        Document fooDoc = new Document("fooDoc");
        fooDoc.addKey(fooKey);
        fooDoc.addKey(fooKey2);
        fooDoc.addKey(barKey);
        fooDoc.swig().stringProperty("fileName", "The Foo Doc.txt");

        List<String> groups = fooDoc.getGroups();

        for (String group : groups) {
            System.out.println("Group -> " + group);

            List<StringKey> groupKeys = fooDoc.getGroupKeys(group);
            System.out.println("Keys for group: " + group);
            for (StringKey k : groupKeys) {
                System.out.println("\t-> id: " + k.getId() + " string: " + k.getString() + " (group: " + k.getGroup() + ")");
            }
            System.out.println();
        }

        Indexer indexer = new Indexer();
        indexer.indexDocument(fooDoc);

        List<SearchResult> dont_find_me = indexer.search("dont find me");
        assert (dont_find_me.size() == 0);

        List<SearchResult> findMe1 = indexer.search(":fooGroup fooKey");
        List<SearchResult> findMe2 = indexer.search(":barGroup fooKey2");
        List<SearchResult> findMe3 = indexer.search(":barGroup barKey");

        System.out.println("findMe1 -> " + findMe1.size() + " results");
        System.out.println("\\_ fileName: " + findMe1.get(0).getDocument().stringProperty("fileName"));
        System.out.println("findMe2 -> " + findMe2.size() + " results");
        System.out.println("findMe3 -> " + findMe3.size() + " results");
        System.out.println("\\_ fileName: " + findMe3.get(0).getDocument().stringProperty("fileName"));
        System.out.println(fooDoc.equals(findMe1.get(0).getDocument()));
        System.out.println(fooDoc.equals(findMe3.get(0).getDocument()));
    }
}