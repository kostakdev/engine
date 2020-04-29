find_package(Check REQUIRED)

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckTypeSize)

include_directories(${kostak_SOURCE_DIR}/include 
                    ${kostak_SOURCE_DIR}/logc/src
                    ${kostak_SOURCE_DIR}/rximap/src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/tests)

add_executable(test_netlink_util test_netlink_util.c)
target_link_libraries(test_netlink_util rxilogc ${CHECK_LIBRARIES} pthread kostak_core)

add_test(NAME test_netlink_util 
         COMMAND test_netlink_util 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Testing)

set_tests_properties(test_netlink_util PROPERTIES TIMEOUT 30) 