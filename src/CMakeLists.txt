include_directories(${kostak_SOURCE_DIR}/include 
                    ${kostak_SOURCE_DIR}/logc/src
                    ${kostak_SOURCE_DIR}/umm_malloc/src
                    ${kostak_SOURCE_DIR}/jsmn)

set(SOURCES main.c child.c 
            param.c mount_ns.c 
            netlink_util.c net_eth.c 
            net_forward.c util.c)

add_compile_options(-Wall -Wextra -pedantic -Werror -fstack-protector-strong -fPIE)

if (CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
  add_compile_options( -D__FORTIFY_SOURCE)
endif ()

add_library(kostak_core STATIC ${SOURCES})
add_executable(kostak main.c)

if(KOSTAK_STATIC)
add_compile_options(-static)
target_link_libraries(kostak -static)
endif(KOSTAK_STATIC)

if(KOSTAK_BIG_STACK)
add_compile_options(-DKOSTAK_BIG_STACK)
endif(KOSTAK_BIG_STACK)

target_link_libraries(kostak_core rxilogc umm_malloc)
target_link_libraries(kostak rxilogc umm_malloc kostak_core)

set_property(TARGET kostak PROPERTY POSITION_INDEPENDENT_CODE ON)
