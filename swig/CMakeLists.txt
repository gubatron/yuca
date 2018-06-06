cmake_minimum_required(VERSION 3.7)
project(yuca_jni CXX)

set(SOURCE_FILES
    yuca_java_wrap.cxx)

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
#        -Weverything
        -Wno-c++98-compat
        -Wno-padded
        -Wno-global-constructors
        -Wno-exit-time-destructors
        -Wno-weak-vtables)
endif()

include_directories(
  $ENV{YUCA_ROOT}/src)

add_library(libyuca_shared SHARED IMPORTED)
set_property(TARGET libyuca_shared PROPERTY IMPORTED_LOCATION "./libyuca_shared.dylib")

add_library(yuca_jni_shared SHARED ${SOURCE_FILES})

set_target_properties(yuca_jni_shared PROPERTIES LINKED_LANGUAGE CXX)
target_compile_features(yuca_jni_shared PUBLIC cxx_std_11)

find_package(JNI REQUIRED)
target_include_directories(yuca_jni_shared PUBLIC ${JNI_INCLUDE_DIRS})

#using darwin : x86_64 : clang++ :
#    <cxxflags>-std=c++11
#    <cxxflags>-mmacosx-version-min=10.9
#    <cxxflags>-fno-strict-aliasing
#    <cxxflags>-fvisibility=hidden
#    <cxxflags>-D_DARWIN_C_SOURCE
#    ;

target_link_libraries(yuca_jni_shared libyuca_shared)
