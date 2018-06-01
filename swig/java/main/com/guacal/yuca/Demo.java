package com.guacal.yuca;

import com.guacal.yuca.swig.yuca;

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
    }
}