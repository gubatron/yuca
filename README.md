# yuca

Yuca: A light-weight, in-memory, fast and simple to use search engine library.

If your app can't handle installing full featured and heavy engines like Lucene, nor you want to depend on a SQL database for indexing and doing simple search based strings you can use Yuca to index documents under any number of arbitrary keys which can be grouped under tags.

LIBRARY BINARY DISTRIBUTION GOALS:
 - static library: yuca.lib (windows x64), yuca.a (linux x64, mac x64)
 - shared library: yuca.dll (windows x64), yuca.so (linux x64, android arm, android arm64), yuca.dylib (mac x64)

LIBRARY BINDING COMPATIBILITY GOALS:
 - JNI/Java SE bindings, yuca.jar (windows x64, linux x64, mac x64)
 - JNI/Android bindings, yuca.jar (arm, arm64)
 - Javascript/NodeJS bindings
 - Python bindings
 - Go bindings
 - PHP bindings
