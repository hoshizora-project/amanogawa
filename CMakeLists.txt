cmake_minimum_required(VERSION 3.7)
project(nagare)

#set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
#set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_library(from_csv SHARED src/nagare/plugin/extract/from_csv.cpp)
add_library(add SHARED src/nagare/plugin/transform/add.cpp)
add_library(to_plain SHARED src/nagare/plugin/load/to_plain.cpp)

set(SOURCE_FILES src/nagare/main.cpp)
add_subdirectory(src/external/pybind11)
#add_subdirectory(src/external/arrow/cpp) # need manual build...
INCLUDE_DIRECTORIES(src src/external/arrow/cpp/src)

if (UNIX AND NOT APPLE)
    # linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(LIBS_EXT a)
else ()
    # macos
    set(LIBS_EXT dylib)
endif ()

#add_executable(nagare ${SOURCE_FILES})
pybind11_add_module(nagare SHARED src/nagare/main.cpp)
find_library(LIB_ARROW NAMES libarrow.${LIBS_EXT} PATHS src/external/arrow/cpp/debug/debug)
target_link_libraries(nagare dl ${LIB_ARROW})
