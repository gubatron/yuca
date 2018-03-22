cmake_minimum_required(VERSION 3.7)
project(yuca CXX)

# if you need to add more library src/ folders, add them here.
include_directories(src
                    ${LIBTORRENT_ROOT}/include)

# if you need to add a library -L folder add it here
link_directories(${LIBTORRENT_ROOT})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
set(CMAKE_CXX_STANDARD 11)
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

# the library libyuca
add_library(yuca STATIC ${SOURCE_FILES})

# demo executable to show how to use the library
add_executable(yuca_demo demo.cpp)
# had to add this because this target has no .cpp files and otherwise it errors out
set_target_properties(yuca PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(yuca_demo yuca)

# unit test with catch 2
set(TEST_FILES
    tests/catch.hpp
    #ests/document_tests.cpp
    #ests/indexer_tests.cpp
    tests/all_tests.cpp
    src/yuca/document.cpp
    src/yuca/types.hpp)

add_executable(yuca_tests ${SOURCE_FILES} ${TEST_FILES})
set_target_properties(yuca_tests PROPERTIES LINKER_LANGUAGE CXX)


# ////////////////////////////////////////////////////////////////////////////////

# a cplus plus playground area for random experiments to interactively debug ideas or APIs we might use.
#add_executable(yuca_playground playground.cpp)

# if you need to link a library 'libfoo' (-lfoo) to test with yuca_playground, use
# target_link_libraries(yuca_playground foo)
