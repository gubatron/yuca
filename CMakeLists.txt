cmake_minimum_required(VERSION 3.6)
project(yuca)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBTORRENT_ROOT $ENV{LIBTORRENT_ROOT} CACHE STRING "libtorrent root")
include_directories(${LIBTORRENT_ROOT}/include
                    include)
link_directories(${LIBTORRENT_ROOT})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/version.cpp
        src/indexer.cpp src/document.cpp include/yuca/document.hpp)

add_library(yuca STATIC ${SOURCE_FILES})

add_executable(yuca_test main.cpp)
target_link_libraries(yuca_test torrent-rasterbar yuca)
