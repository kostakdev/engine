cmake_minimum_required(VERSION 3.1)

project(kostak C)

set(BUILD_SHARED_LIBS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_RELEASE} -s")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_compile_options(-ffile-prefix-map=../src/=)

set(LOG_SOURCES ${PROJECT_SOURCE_DIR}/logc/src/log.c)
add_library(rxilogc STATIC ${LOG_SOURCES})

set(UMM_MALLOC_SOURCEDIR ${PROJECT_SOURCE_DIR}/umm_malloc/src/)
set(UMM_MALLOC_SOURCES
    ${UMM_MALLOC_SOURCEDIR}/umm_info.c
    ${UMM_MALLOC_SOURCEDIR}/umm_integrity.c
    ${UMM_MALLOC_SOURCEDIR}/umm_malloc.c
    ${UMM_MALLOC_SOURCEDIR}/umm_poison.c)

add_library(umm_malloc STATIC ${UMM_MALLOC_SOURCES})
target_include_directories(umm_malloc PRIVATE 
  ${PROJECT_SOURCE_DIR}/include 
  ${PROJECT_SOURCE_DIR}/logc/src)

add_subdirectory(src)

if (NOT CMAKE_CROSSCOMPILING)
  enable_testing()
  add_subdirectory(tests)
endif (NOT CMAKE_CROSSCOMPILING) 
