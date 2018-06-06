#!/bin/bash

${YUCA_ROOT?"Error: YUCA_ROOT environment variable is not set"}

# TODO: once java is done, pack everything into functions and parametrize this build script
# so we can build bindings for each different language.

# Clear everything
JAVA_SRC_OUTPUT=java/main/com/guacal/yuca/swig
rm -rf ${JAVA_SRC_OUTPUT}
mkdir -p ${JAVA_SRC_OUTPUT}
rm *.cxx *.dylib *.so *.dll *.cmake

rm -fr CMakeFiles
rm CMakeCache.txt Makefile

# Run swig to create .cxx and .java wrappers
swig -c++ -java \
     -package com.guacal.yuca.swig \
     -outdir ${JAVA_SRC_OUTPUT} \
     -I${YUCA_ROOT}/src \
     -v yuca_java.i 

# possible swig options to add:
#-outdir java/src/main/org/guacal/yuca/swig
#-package org.guacal.yuca.swig
#-DSOME_OPTION="value"

# Copy libraries here
SHARED_LIB_EXT='.dylib'
case "$(uname -s)" in
    Darwin) SHARED_LIB_EXT='dylib';;
    Linux)  SHARED_LIB_EXT='so';;
    CYGWIN*|MINGW*) SHARED_LIB_EXT='dll';;
esac

if [ ! -f ../libyuca_shared.${SHARED_LIB_EXT} ]; then
    echo
    echo "Error: Missing libyuca_shared.${SHARED_LIB_EXT} library"
    echo
    echo "Please build libyuca_shared.${SHARED_LIB_EXT} before building language bindings"
    echo "Try:"
    echo "$ cd ..; cmake .; make -j4; cd swig; ./build.sh"
    echo
    echo
  exit 1
fi

export SHARED_LIB_EXT

make clean

#TODO (maybe) have a different Cmake file for each language or parametrized cmake, e.g. CMakeLists_java.txt, CMakeLists_js.txt, CMakeLists_python.txt
cmake .
make

gradle build
./run
