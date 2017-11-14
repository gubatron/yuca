cmake_minimum_required(VERSION 3.8)
project(yuca CXX)

if(NOT DEFINED ENV{GTEST_DIR})
    message(FATAL_ERROR "GTEST_DIR environment variable not defined.\nFix your .bashrc or .bash_profile with something like:\n  export GTEST_DIR=/home/johndoe/googletest/googletest\nYou can get googletest at https://github.com/google/googletest/")
endif()

set(GTEST_DIR $ENV{GTEST_DIR} CACHE STRING "googletest root")

# if you need to add more library src/ folders, add them here.
include_directories(src
                    ${GTEST_DIR}/include
                    ${LIBTORRENT_ROOT}/include)

# if you need to add a library -L folder add it here
link_directories(${LIBTORRENT_ROOT}
                 ${GTEST_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

set(CMAKE_BUILD_TYPE Debug)
set(SOURCE_FILES
        src/yuca/yuca.cpp #gotta have at least one .cpp file, otherwise the compiler/linker freaks out
        src/yuca/types.hpp
        src/yuca/key.hpp
        src/yuca/key.cpp
        src/yuca/document.hpp
        src/yuca/document.cpp
        src/yuca/indexer.hpp
        src/yuca/indexer.cpp
        )

set(TEST_FILES
        tests/all_tests.cpp src/yuca/document.cpp src/yuca/types.hpp)

# the library libyuca
add_library(yuca STATIC ${SOURCE_FILES})

# demo executable to show how to use the library
add_executable(yuca_demo demo.cpp)
# had to add this because this target has no .cpp files and otherwise it errors out
set_target_properties(yuca PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(yuca_demo yuca)

# ////////////////////////////////////////////////////////////////////////////////
# unit tests with google test, which require pthread
# ////////////////////////////////////////////////////////////////////////////////
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(yuca_tests ${SOURCE_FILES} ${TEST_FILES})
set_target_properties(yuca_tests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(yuca_tests Threads::Threads gtest yuca)
# ////////////////////////////////////////////////////////////////////////////////

# a cplus plus playground area for random experiments to interactively debug ideas or APIs we might use.
add_executable(yuca_playground playground.cpp)

# if you need to link a library 'libfoo' (-lfoo) to test with yuca_playground, use
# target_link_libraries(yuca_playground foo)
