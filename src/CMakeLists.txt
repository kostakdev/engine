include_directories(${kostak_SOURCE_DIR}/include 
                    ${kostak_SOURCE_DIR}/logc/src
                    ${kostak_SOURCE_DIR}/rximap/src)

set(SOURCES main.c child.c 
            param.c mount_ns.c 
            netlink_util.c net_eth.c 
            net_forward.c util.c)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_library(kostak_core STATIC ${SOURCES})
add_executable(kostak main.c)

if(KOSTAK_STATIC)
target_link_libraries(kostak -static)
endif(KOSTAK_STATIC)


target_link_libraries(kostak_core rxilogc)
target_link_libraries(kostak rxilogc rximap kostak_core)
