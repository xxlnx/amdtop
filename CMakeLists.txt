cmake_minimum_required(VERSION 3.1)
project(XXXTool C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

execute_process(COMMAND git describe --abbrev=12 --always
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_REV)
add_definitions(-DBUILD_GIT_VERSION=\"${GIT_REV}\")

include_directories(./)

add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(tui)