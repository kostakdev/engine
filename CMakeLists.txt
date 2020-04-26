cmake_minimum_required(VERSION 3.1)

project(kostak)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options(-ffile-prefix-map=${PROJECT_SOURCE_DIR}/=)

set(SOURCES main.c child.c param.c mount_ns.c env.c netlink_util.c net_eth.c net_forward.c)
set(LOG_SOURCES ${PROJECT_SOURCE_DIR}/logc/src/log.c)

add_library(logc STATIC ${LOG_SOURCES})

add_executable(kostak ${SOURCES})

target_link_libraries(kostak logc)
target_include_directories(kostak PUBLIC
  "${PROJECT_SOURCE_DIR}/logc/src")