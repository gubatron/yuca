#!/bin/bash

# TODO: once java is done, pack everything into functions and parametrize this build script
# so we can build bindings for each different language.

# Clear everything
JAVA_SRC_OUTPUT=java/main/com/guacal/yuca/swig
rm -rf ${JAVA_SRC_OUTPUT}
mkdir -p ${JAVA_SRC_OUTPUT}
rm *.cxx *.dylib *.cmake
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
# TODO: verify current OS and copy right library
cp ../libyuca_shared.dylib .

make clean
#TODO have a different Cmake file for each language, e.g. CMakeLists_java.txt, CMakeLists_js.txt, CMakeLists_python.txt
cmake .
make
