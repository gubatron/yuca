cmake_minimum_required(VERSION 3.7)
project(yuca CXX)

include_directories(src)

set(SOURCE_FILES
        src/yuca/yuca.hpp
        src/yuca/types.hpp
        src/yuca/key.hpp
        src/yuca/key.cpp
        src/yuca/document.hpp
        src/yuca/document.cpp
        src/yuca/indexer.hpp
        src/yuca/indexer.cpp
        src/yuca/utils.hpp
        )

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Weverything
        -Wno-c++98-compat
        -Wno-padded
        -Wno-global-constructors
        -Wno-exit-time-destructors
        -Wno-weak-vtables)
endif()

# the libyuca_* libraries
add_library(yuca_shared SHARED ${SOURCE_FILES})
add_library(yuca_static STATIC ${SOURCE_FILES})

set_target_properties(yuca_shared PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(yuca_static PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features(yuca_shared PUBLIC cxx_std_11)
target_compile_features(yuca_static PUBLIC cxx_std_11)

# demo executable to show how to use the library
add_executable(yuca_demo_shared demo.cpp)
add_executable(yuca_demo_static demo.cpp)

target_link_libraries(yuca_demo_shared yuca_shared)
target_link_libraries(yuca_demo_static yuca_static)

# unit tests with catch 2 (files are checked in the order they are declared, the ones on top first)
set(TEST_FILES
        tests/utils_tests.cpp
        tests/document_tests.cpp
        tests/indexer_tests.cpp
        tests/tests_main.cpp)

add_executable(yuca_tests ${SOURCE_FILES} ${TEST_FILES})
set_target_properties(yuca_tests PROPERTIES LINKER_LANGUAGE CXX)
# TODO: remove this weh tests link to the library
target_compile_features(yuca_tests PUBLIC cxx_std_11)
