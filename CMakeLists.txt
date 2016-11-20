cmake_minimum_required(VERSION 3.6)
project(yuca)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBTORRENT_ROOT $ENV{LIBTORRENT_ROOT} CACHE STRING "libtorrent root")

# if you need to add more library include/ folders, add them here.
include_directories(${LIBTORRENT_ROOT}/include
                    include
                    ${GTEST_DIR}/include)

# if you need to add a library -L folder add it here
link_directories(${LIBTORRENT_ROOT}
                 ${GTEST_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/version.cpp
        src/indexer.cpp
        src/document.cpp
        include/yuca/document.hpp
        include/yuca/key.h
        include/yuca/yuca.h)

set(TEST_FILES
        test/document_test.cpp
        include/yuca/yuca.h)

# the library libyuca
add_library(yuca STATIC ${SOURCE_FILES})

# demo executable to show how to use the library
add_executable(yuca_demo demo.cpp)
target_link_libraries(yuca_demo torrent-rasterbar yuca)

# unit tests
add_executable(yuca_tests ${TEST_FILES} ${SOURCE_FILES})
target_link_libraries(yuca_tests torrent-rasterbar yuca)

# a cplus plus playground area for random experiments to interactively debug ideas or APIs we might use.
add_executable(yuca_playground playground.cpp playground.cpp)

# if you need to link a library 'libfoo' (-lfoo) to test with yuca_playground, use
# target_link_libraries(yuca_playground foo)
