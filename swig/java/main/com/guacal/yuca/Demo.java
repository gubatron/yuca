package com.guacal.yuca;

import com.guacal.yuca.swig.yuca;

public final class Demo {
    public static void main(String[] args) {
        System.out.println("Yuca Java Wrapper version: " + yuca.version() + "\nLast Updated: " yuca.getLAST_UPDATED() + "\n\n");
    }
}