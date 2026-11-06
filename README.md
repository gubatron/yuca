# yuca v0.1.1
<img src="https://i.imgur.com/zZvZ9kK.png"/>

**Yuca: A light-weight, in-memory, fast and simple to use search engine library.**

**Start here:** [User manual](docs/manual.md) · [Development, benchmarks, and fuzz testing](docs/development.md) · [Changelog](CHANGELOG.md) · [C++ engineering skill](skills/cpp-engineer/SKILL.md)

If your app can't handle or doesn't really need installing a full featured and heavy search engine like Lucene, nor you want to depend on a SQL database for indexing and doing simple search based strings you can use Yuca to index documents under any number of arbitrary keys which can be grouped under tags.

The shared library currently weighs ~170kb without any packing optimizations, we hope to reduce the size further in the near future.

## BUILDING

Requirements:
 - CMake 3.16 or newer, and a supported build tool (Make or Ninja)
 - C++11 compiler (clang recommended)

Instructions:

Configure and build out of source (no dependencies are downloaded):
```
$ cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
```
then after that just build
```
$ cmake --build build/release --parallel 4
$ ctest --test-dir build/release --output-on-failure
```

Shared/static libraries, `yuca_demo_shared`, `yuca_demo_static`, and `yuca_tests` are created in `build/release`. CTest exercises both linking modes and the unit tests. Install libraries and public headers with `cmake --install build/release --prefix /your/prefix`.

There's a also `yuca_tests` executable in case you want to make sure all the unit tests are passing.

## BUILDING JAVA (SWIG/JNI) WRAPPER

The `swig` folder holds swig scripts and high level wrappers to use yuca from other programming languages, the first being `java`.

The current intention is to have the `swig/build.sh` be a one step build process to create bindings for other languages.

For now it's a very rough script that will create java bindings, currently supports bindings for Java on MacOS since we're still early in the development of the bindings. In the long run, options will be passed to build.sh to output bindings for the language and architecture of your choice.

The JNI scripts are legacy and currently assume libraries in the repository root; they have **not** been updated or verified with the new out-of-source build. The commands below describe the historical workflow, not a qualified build path:

```
$ cd swig
$ ./build.sh
```

This will output programatically created C++ JNI code and Java code to access the JNI native wrappers inside a `swig/java` folder. In there we have a java package which is Java Programmer friendly `com.guacal.yuca.*` In there you will find the main classes to use Yuca in your Java/Android app `Indexer`, `Document`, `StringKey`, `SearchRequest`, `SearchResult`.

## Debug Build

Use a separate build directory; do not edit the build script:
```sh
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DYUCA_ENABLE_SANITIZERS=ON
cmake --build build/debug --parallel 4
ctest --test-dir build/debug --output-on-failure
```

AddressSanitizer and UndefinedBehaviorSanitizer require Clang or GCC. See [the development manual](docs/development.md) for benchmarks and fuzz testing.


 
## Feedback and patches welcome
 
 Please open an issue on the github issue tracker if you have any problems, ideas.
 If you feel like extending or patching, please open an issue first so that you don't waste your time coding something that might not be accepted for a good reason.

## Index checkpoints

Save the whole index with `index.save("catalog.yuca")` and resume with `auto index = yuca::Indexer::load("catalog.yuca")`.
Binary byte-buffer serialization is also available. See the [checkpoint manual](docs/manual.md#saving-and-restoring-yuca-checkpoints)
for compatibility, limits, atomic replacement, and a compiled example. Incremental index work after loading is supported;
incremental on-disk updates/a write-ahead log are not yet implemented.

### LIBRARY BINARY DISTRIBUTION GOALS:
 - static library: yuca.lib (windows x64), yuca.a (linux x64, mac x64)
 - shared library: yuca.dll (windows x64), yuca.so (linux x64, android arm, android arm64), yuca.dylib (mac x64)

### LIBRARY BINDING COMPATIBILITY GOALS:
 - JNI/Java SE bindings, yuca.jar (windows x64, linux x64, mac x64)
 - JNI/Android bindings, yuca.jar (arm, arm64)
 - Javascript/NodeJS bindings
 - Python bindings
 - Go bindings
 - PHP bindings
 
 *This project is available under The MIT License, Released publicly Wed. May 9th of 2018*
