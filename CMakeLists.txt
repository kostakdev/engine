cmake_minimum_required(VERSION 3.1)

project(kostak)

set(BUILD_SHARED_LIBS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_compile_options(-ffile-prefix-map=../src/=)

set(LOG_SOURCES ${PROJECT_SOURCE_DIR}/logc/src/log.c)
add_library(rxilogc STATIC ${LOG_SOURCES})

set(MAP_SOURCES ${PROJECT_SOURCE_DIR}/rximap/src/map.c)
add_library(rximap STATIC ${MAP_SOURCES})

add_subdirectory(src)

enable_testing()

add_subdirectory(tests)
