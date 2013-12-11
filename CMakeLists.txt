cmake_minimum_required(VERSION 2.8)

set(COMMON_SOURCES ../common/common.cpp)

add_definitions("-Wall -Wextra")
add_definitions("-std=c++11")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIR})

add_subdirectory(tpie)
add_subdirectory(stxxl)
