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

        StringKey fooKey = new StringKey("fooKey",":fooTag");
        StringKey fooKey2 = new StringKey("fooKey2",":fooTag");
        StringKey barKey = new StringKey("barKey", ":barTag");

        System.out.println("<" + barKey.getId() + ":" + barKey.getString() + " tag= " + barKey.getTag() + ">");

        Document fooDoc = new Document("fooDoc");
        fooDoc.addKey(fooKey);
        fooDoc.addKey(fooKey2);
        fooDoc.addKey(barKey);
        fooDoc.swig().stringProperty("fileName", "The Foo Doc.txt");

        List<String> tags = fooDoc.getTags();

        for (String tag : tags) {
            System.out.println("Tag -> " + tag);

            List<StringKey> tagKeys = fooDoc.getTagKeys(tag);
            System.out.println("Keys for tag: " + tag);
            for (StringKey k : tagKeys) {
                System.out.println("\t-> id: " + k.getId() + " string: " + k.getString() + " (tag: " + k.getTag() + ")");
            }
            System.out.println();
        }
    }
}