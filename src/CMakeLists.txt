find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBSECCOMP REQUIRED libseccomp)
pkg_check_modules(LIBCAP REQUIRED libcap)

include_directories(${kostak_SOURCE_DIR}/include 
                    ${kostak_SOURCE_DIR}/logc/src
                    ${kostak_SOURCE_DIR}/umm_malloc/src
                    ${kostak_SOURCE_DIR}/jsmn)

set(SOURCES main.c child.c 
            param.c mount_ns.c 
            netlink_util.c net_eth.c 
            net_forward.c util.c cap.c seccomp_filter.c)

add_compile_options(-Wall -Wextra -pedantic -Werror -fstack-protector-strong -fstack-clash-protection -fexceptions -pipe)
if(CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
  add_definitions(-D_FORTIFY_SOURCE=2)
endif(CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
add_library(kostak_core STATIC ${SOURCES})
add_executable(kostak main.c)

if(KOSTAK_BIG_STACK)
add_compile_options(-DKOSTAK_BIG_STACK)
endif(KOSTAK_BIG_STACK)

target_link_libraries(kostak_core rxilogc umm_malloc)
target_link_libraries(kostak rxilogc umm_malloc kostak_core ${LIBCAP_LIBRARIES} ${LIBSECCOMP_LIBRARIES})
target_include_directories(kostak PUBLIC ${LIBCAP_INCLUDE_DIRS} ${LIBSECCOMP_INCLUDE_DIRS})
target_compile_options(kostak PUBLIC ${LIBCAP_CFLAGS_OTHER} ${LIBSECCOMP_CFLAGS_OTHER})

if(KOSTAK_STATIC)
  target_compile_options(kostak PUBLIC -static)
endif(KOSTAK_STATIC)

set_property(TARGET kostak PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_options(kostak PUBLIC "LINKER:-z,defs" "LINKER:-z,relro" "LINKER:-z,now")


