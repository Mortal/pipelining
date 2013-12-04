cmake_minimum_required(VERSION 2.8)

set(COMMON_SOURCES ../common/common.cpp)

add_definitions("-Wall -Wextra")
add_definitions("-std=c++11")

add_subdirectory(tpie)
add_subdirectory(stxxl)
