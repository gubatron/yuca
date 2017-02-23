cmake_minimum_required(VERSION 3.6)
project(yuca CXX)

set(LIBTORRENT_ROOT $ENV{LIBTORRENT_ROOT} CACHE STRING "libtorrent root")
set(GTEST_DIR $ENV{GTEST_DIR} CACHE STRING "googletest root")

# if you need to add more library include/ folders, add them here.
include_directories(include
                    ${GTEST_DIR}/include
                    ${LIBTORRENT_ROOT}/include)

# if you need to add a library -L folder add it here
link_directories(${LIBTORRENT_ROOT}
                 ${GTEST_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        include/yuca/yuca.cpp #gotta have at least one .cpp file, otherwise the compiler/linker freaks out
        include/yuca/key.hpp
        include/yuca/document.hpp
        include/yuca/indexer.hpp
        )

set(TEST_FILES
        tests/all_tests.cpp)

# the library libyuca
add_library(yuca STATIC ${SOURCE_FILES})

# demo executable to show how to use the library
add_executable(yuca_demo demo.cpp)
# had to add this because this target has no .cpp files and otherwise it errors out
set_target_properties(yuca PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(yuca_demo torrent-rasterbar yuca)

# ////////////////////////////////////////////////////////////////////////////////
# unit tests with google test, which require pthread
# ////////////////////////////////////////////////////////////////////////////////
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(yuca_tests ${SOURCE_FILES} ${TEST_FILES})
set_target_properties(yuca_tests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(yuca_tests torrent-rasterbar Threads::Threads gtest yuca)
# ////////////////////////////////////////////////////////////////////////////////

# a cplus plus playground area for random experiments to interactively debug ideas or APIs we might use.
add_executable(yuca_playground playground.cpp)

# if you need to link a library 'libfoo' (-lfoo) to test with yuca_playground, use
# target_link_libraries(yuca_playground foo)
