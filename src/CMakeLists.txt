include_directories(${kostak_SOURCE_DIR}/include 
                    ${kostak_SOURCE_DIR}/logc/src
                    ${kostak_SOURCE_DIR}/rximap/src)

set(SOURCES main.c child.c param.c mount_ns.c env.c netlink_util.c net_eth.c net_forward.c)

add_executable(kostak ${SOURCES})

if(KOSTAK_STATIC)
target_link_libraries(kostak -static)
endif(KOSTAK_STATIC)

target_link_libraries(kostak rxilogc rximap)