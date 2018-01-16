cmake_minimum_required(VERSION 3.7)
project(amanogawa)

#set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
#set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wno-return-type-c-linkage -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(PYTHON_VERSION 3.6)
set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION})
set(PYTHON_EXECUTABLE python${PYTHON_VERSION})

if (UNIX AND NOT APPLE)
    # linux
    set(LIBS_EXT so)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
else ()
    # macos
    set(LIBS_EXT dylib)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBS_EXT=${LIBS_EXT}")


# dependencies
## pybind11
add_subdirectory(src/pybind11)

## numpy
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/arrow/cpp/cmake_modules)
find_package(NumPy REQUIRED)

# python
find_package(PythonLibsNew REQUIRED)

## arrow
find_library(LIB_ARROW NAMES arrow PATHS src/arrow/cpp/build/debug)
find_library(LIB_ARROW_PYTHON NAMES arrow_python PATHS src/arrow/cpp/build/debug)

file(GLOB SOURCES src/amanogawa/**/*.h)
include_directories(src)
include_directories(SYSTEM
        ${NUMPY_INCLUDE_DIRS}
        src/arrow/cpp/src
        src/cpptoml/include
        src/spdlog/include
        src/text-csv/include
        src/pybind11/include)


# plugins
## source
### file
set(AMANOGAWA_PLUGIN_SOURCE_FILE source_file)
add_library(${AMANOGAWA_PLUGIN_SOURCE_FILE} SHARED src/amanogawa/plugin/source/file/file.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_SOURCE_FILE} ${LIB_ARROW})

## flow
### filter
set(AMANOGAWA_PLUGIN_FLOW_FILTER flow_filter)
add_library(${AMANOGAWA_PLUGIN_FLOW_FILTER} SHARED src/amanogawa/plugin/flow/filter/filter.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_FLOW_FILTER} ${LIB_ARROW})

### to_graph
set(AMANOGAWA_PLUGIN_FLOW_TO_GRAPH flow_to_graph)
add_library(${AMANOGAWA_PLUGIN_FLOW_TO_GRAPH} SHARED src/amanogawa/plugin/flow/to_graph/to_graph.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_FLOW_TO_GRAPH} ${LIB_ARROW})

## sink
### file
set(AMANOGAWA_PLUGIN_SINK_FILE sink_file)
add_library(${AMANOGAWA_PLUGIN_SINK_FILE} SHARED src/amanogawa/plugin/sink/file/file.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_SINK_FILE} ${LIB_ARROW})

### numpy
set(AMANOGAWA_PLUGIN_SINK_NUMPY sink_numpy)
pybind11_add_module(${AMANOGAWA_PLUGIN_SINK_NUMPY} SHARED src/amanogawa/plugin/sink/numpy/numpy.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_SINK_NUMPY} ${LIB_ARROW} ${LIB_ARROW_PYTHON} ${PYTHON_LIBRARIES})

set_target_properties(
        ${AMANOGAWA_PLUGIN_SOURCE_FILE}
        ${AMANOGAWA_PLUGIN_FLOW_FILTER}
        ${AMANOGAWA_PLUGIN_FLOW_TO_GRAPH}
        ${AMANOGAWA_PLUGIN_SINK_FILE}
        ${AMANOGAWA_PLUGIN_SINK_NUMPY}
        PROPERTIES PREFIX lib SUFFIX .${LIBS_EXT})


# cli
set(AMANOGAWA_CLI amanogawa-cli)
add_executable(${AMANOGAWA_CLI} ${SOURCES} src/amanogawa/cli.cpp)
target_link_libraries(${AMANOGAWA_CLI} dl ${LIB_ARROW} ${LIB_ARROW_PYTHON} pybind11::embed)


# pybind
set(AMANOGAWA amanogawa)
pybind11_add_module(${AMANOGAWA} SHARED ${SOURCES} src/amanogawa/pybind.cpp)
target_link_libraries(${AMANOGAWA} dl ${LIB_ARROW} ${LIB_ARROW_PYTHON})
set_target_properties(${AMANOGAWA} PROPERTIES SUFFIX .${LIBS_EXT})
