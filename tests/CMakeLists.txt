find_package(PkgConfig REQUIRED)

pkg_check_modules(Check REQUIRED check) 

include_directories(${kostak_SOURCE_DIR}/include 
                    ${kostak_SOURCE_DIR}/logc/src
                    ${kostak_SOURCE_DIR}/umm_malloc/src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/tests)

add_executable(test_netlink_util test_netlink_util.c)
target_compile_options(test_netlink_util PUBLIC ${Check_CFLAGS_OTHER})
target_include_directories(test_netlink_util PUBLIC ${Check_INCLUDE_DIRS})
target_link_libraries(test_netlink_util rxilogc ${Check_LIBRARIES} pthread kostak_core)

add_test(NAME test_netlink_util 
         COMMAND test_netlink_util 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Testing)

set_tests_properties(test_netlink_util PROPERTIES TIMEOUT 30) 
