cmake_minimum_required(VERSION 3.6)
project(yuca CXX)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBTORRENT_ROOT $ENV{LIBTORRENT_ROOT} CACHE STRING "libtorrent root")
set(GTEST_DIR $ENV{GTEST_DIR} CACHE STRING "googletest root")

# if you need to add more library include/ folders, add them here.
include_directories(${LIBTORRENT_ROOT}/include
                    include
                    ${GTEST_DIR}/include)

# /Users/gubatron/workspace.frostwire/googletest/googletest

# if you need to add a library -L folder add it here
link_directories(${LIBTORRENT_ROOT}
                 ${GTEST_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        include/yuca/yuca.hpp
        include/yuca/document.hpp
        include/yuca/key.hpp
        )

set(TEST_FILES
        tests/document_tests.cpp
        include/yuca/yuca.hpp)

# the library libyuca
add_library(yuca STATIC ${SOURCE_FILES} src/version.cpp)

# demo executable to show how to use the library
add_executable(yuca_demo demo.cpp)
target_link_libraries(yuca_demo torrent-rasterbar yuca)

# ////////////////////////////////////////////////////////////////////////////////
# unit tests with google test, which require pthread
# ////////////////////////////////////////////////////////////////////////////////
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(yuca_tests ${SOURCE_FILES} ${TEST_FILES})
target_link_libraries(yuca_tests torrent-rasterbar Threads::Threads gtest yuca)
# ////////////////////////////////////////////////////////////////////////////////

# a cplus plus playground area for random experiments to interactively debug ideas or APIs we might use.
add_executable(yuca_playground playground.cpp)

# if you need to link a library 'libfoo' (-lfoo) to test with yuca_playground, use
# target_link_libraries(yuca_playground foo)
