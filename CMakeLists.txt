cmake_minimum_required(VERSION 3.1)

project(kostak)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options(-ffile-prefix-map=${PROJECT_SOURCE_DIR}/=)

set(LOG_SOURCES ${PROJECT_SOURCE_DIR}/logc/src/log.c)
add_library(logc STATIC ${LOG_SOURCES})

add_subdirectory(src)