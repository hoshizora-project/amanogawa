cmake_minimum_required(VERSION 3.7)
project(amanogawa)

#set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
#set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wno-return-type-c-linkage -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (UNIX AND NOT APPLE)
    # linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(LIBS_EXT a)
else ()
    # macos
    set(LIBS_EXT dylib)
endif ()

# TODO: Separate libamanogawa to reduce duplicated recompilation
file(GLOB SOURCES src/amanogawa/**/*.h)
add_subdirectory(src/pybind11)
#add_subdirectory(src/arrow/cpp) # need manual build...
find_library(LIB_ARROW NAMES libarrow.${LIBS_EXT} PATHS src/arrow/cpp/debug/debug)
include_directories(src src/arrow/cpp/src src/cpptoml/include src/spdlog/include src/text-csv/include)

# plugins
add_library(source_file SHARED src/amanogawa/plugin/source/file/file.cpp)
target_link_libraries(source_file ${LIB_ARROW})
add_library(flow_filter SHARED src/amanogawa/plugin/flow/filter/filter.cpp)
target_link_libraries(flow_filter ${LIB_ARROW})
add_library(flow_to_graph SHARED src/amanogawa/plugin/flow/to_graph/to_graph.cpp)
target_link_libraries(flow_to_graph ${LIB_ARROW})
add_library(sink_file SHARED src/amanogawa/plugin/sink/file/file.cpp)
target_link_libraries(sink_file ${LIB_ARROW})

# cli
add_executable(amanogawa-cli ${SOURCES} src/amanogawa/cli.cpp)
target_link_libraries(amanogawa-cli dl ${LIB_ARROW})

# pybind
pybind11_add_module(amanogawa-py SHARED ${SOURCES} src/amanogawa/pybind.cpp)
target_link_libraries(amanogawa-py dl ${LIB_ARROW})
