cmake_minimum_required(VERSION 3.7)
project(amanogawa)

#set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
#set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wno-return-type-c-linkage -O0 -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# external library
find_library(LIB_ARROW NAMES libarrow.${LIBS_EXT} PATHS src/arrow/cpp/debug/debug)

add_library(source_file SHARED src/amanogawa/plugin/source/file/file.cpp)
target_link_libraries(source_file ${LIB_ARROW})
add_library(flow_example_add SHARED src/amanogawa/plugin/flow/example_add/example_add.cpp)
target_link_libraries(flow_example_add ${LIB_ARROW})
add_library(sink_file SHARED src/amanogawa/plugin/sink/file/file.cpp)
target_link_libraries(sink_file ${LIB_ARROW})

set(SOURCE_FILES src/amanogawa/main.cpp src/amanogawa/core/executor.h src/amanogawa/core/confing.h src/amanogawa/core/column_info.h src/amanogawa/include/common.h src/amanogawa/include/flow_plugin.h src/amanogawa/include/sink_plugin.h src/amanogawa/include/source_plugin.h src/amanogawa/core/any.h src/amanogawa/core/row.h src/amanogawa/core/dl.h src/amanogawa/include/util.h src/amanogawa/include/plugin.h)
add_subdirectory(src/pybind11)
#add_subdirectory(src/arrow/cpp) # need manual build...
INCLUDE_DIRECTORIES(src src/arrow/cpp/src src/cpptoml/include src/spdlog/include)

if (UNIX AND NOT APPLE)
    # linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(LIBS_EXT a)
else ()
    # macos
    set(LIBS_EXT dylib)
endif ()

add_executable(amanogawa-cli ${SOURCE_FILES})
target_link_libraries(amanogawa-cli dl ${LIB_ARROW})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPYTHON")
#pybind11_add_module(amanogawa SHARED ${SOURCE_FILES})
#target_link_libraries(amanogawa dl ${LIB_ARROW})
