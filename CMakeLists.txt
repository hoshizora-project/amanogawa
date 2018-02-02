cmake_minimum_required(VERSION 3.0)
project(amanogawa)

#set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
#set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wno-return-type-c-linkage -DSPDLOG_DEBUG_ON -DDEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-return-type-c-linkage")

set(PYTHON_VERSION 3)
set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION})
execute_process(COMMAND bash -c "echo -n `which python${PYTHON_VERSION}`" OUTPUT_VARIABLE PYTHON_EXECUTABLE)

if (UNIX AND NOT APPLE)
    # linux
    set(LIBS_EXT so)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
else ()
    # macos
    set(LIBS_EXT dylib)
#    include_directories(SYSTEM src/pcm/MacMSRDriver)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBS_EXT=${LIBS_EXT}")

#if (${CMAKE_BUILD_TYPE} MATCHES Debug)
#    ## pcm
#    set(PCM_SOURCES
#            src/pcm/cpucounters.cpp
#            src/pcm/msr.cpp
#            src/pcm/pci.cpp
#            src/pcm/client_bw.cpp)
#    set(LIB_PCM pcm)
#    add_library(${LIB_PCM} STATIC ${PCM_SOURCES})
#endif ()

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
        src/pybind11/include
        src/json/src
        src/pugixml/src)


# plugins
## source
### file
set(AMANOGAWA_PLUGIN_SOURCE_FILE source_file)
add_library(${AMANOGAWA_PLUGIN_SOURCE_FILE} SHARED src/amanogawa/plugin/source/file/file.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_SOURCE_FILE} ${LIB_ARROW})

## format
### csv
set(AMANOGAWA_PLUGIN_FORMAT_CSV format_csv)
add_library(${AMANOGAWA_PLUGIN_FORMAT_CSV} SHARED src/amanogawa/plugin/format/csv/csv.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_FORMAT_CSV} ${LIB_ARROW})

### json
set(AMANOGAWA_PLUGIN_FORMAT_JSON format_json)
add_library(${AMANOGAWA_PLUGIN_FORMAT_JSON} SHARED src/amanogawa/plugin/format/json/json.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_FORMAT_JSON} ${LIB_ARROW})

### xml
set(AMANOGAWA_PLUGIN_FORMAT_XML format_xml)
add_library(${AMANOGAWA_PLUGIN_FORMAT_XML} SHARED src/amanogawa/plugin/format/xml/xml.cpp)
add_subdirectory(src/pugixml)
target_link_libraries(${AMANOGAWA_PLUGIN_FORMAT_XML} ${LIB_ARROW} pugixml)

## flow
### filter
set(AMANOGAWA_PLUGIN_FLOW_FILTER flow_filter)
add_library(${AMANOGAWA_PLUGIN_FLOW_FILTER} SHARED src/amanogawa/plugin/flow/filter/filter.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_FLOW_FILTER} ${LIB_ARROW})

### to_graph
set(AMANOGAWA_PLUGIN_FLOW_TO_GRAPH flow_to_graph)
find_library(LIB_MECAB NAMES mecab)
file(GLOB FLOW_TO_GRAPH_SOURCES src/amanogawa/plugin/flow/to_graph/*.h)
add_library(${AMANOGAWA_PLUGIN_FLOW_TO_GRAPH} SHARED
        ${FLOW_TO_GRAPH_SOURCES} src/amanogawa/plugin/flow/to_graph/to_graph.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_FLOW_TO_GRAPH} ${LIB_ARROW} ${LIB_MECAB})

## branch
set(AMANOGAWA_PLUGIN_BRANCH_COLUMN branch_column)
add_library(${AMANOGAWA_PLUGIN_BRANCH_COLUMN} SHARED src/amanogawa/plugin/branch/column/column.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_BRANCH_COLUMN} ${LIB_ARROW})

## confluence
### key
set(AMANOGAWA_PLUGIN_CONFLUENCE_KEY confluence_key)
add_library(${AMANOGAWA_PLUGIN_CONFLUENCE_KEY} SHARED src/amanogawa/plugin/confluence/key/key.cpp)
target_link_libraries(${AMANOGAWA_PLUGIN_CONFLUENCE_KEY} ${LIB_ARROW})

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
        ${AMANOGAWA_PLUGIN_FORMAT_CSV}
        ${AMANOGAWA_PLUGIN_FORMAT_JSON}
        ${AMANOGAWA_PLUGIN_FORMAT_XML}
        ${AMANOGAWA_PLUGIN_FLOW_FILTER}
        ${AMANOGAWA_PLUGIN_FLOW_TO_GRAPH}
        ${AMANOGAWA_PLUGIN_BRANCH_COLUMN}
        ${AMANOGAWA_PLUGIN_CONFLUENCE_KEY}
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


#if (${CMAKE_BUILD_TYPE} MATCHES Debug)
#    target_link_libraries(${AMANOGAWA_CLI} ${LIB_PCM})
#    target_link_libraries(${AMANOGAWA} ${LIB_PCM})
#endif ()