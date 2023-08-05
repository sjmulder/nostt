# This CMake build file is provided for convenience but the Makefile is
# the sole canonical build file.

cmake_minimum_required(VERSION 3.18.1)
include(FindPkgConfig)

project(nostt C)

find_package(CURL REQUIRED)
pkg_search_module(JSON_C REQUIRED json-c)

add_executable(nostt
    nostt.c
    api.c
    compat.c)

target_include_directories(nostt
    PUBLIC ${JSON_C_INCLUDE_DIRS}
    PUBLIC ${CURL_INCLUDE_DIRS})

target_link_libraries(nostt
    ${JSON_C_LINK_LIBRARIES}
    ${CURL_LIBRARIES})
