find_package(Boost COMPONENTS date_time thread filesystem system regex)

add_subdirectory(tpie)
include_directories(tpie)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/tpie)

add_executable(tpie-transform transform.cpp ${COMMON_SOURCES})
target_link_libraries(tpie-transform ${Boost_LIBRARIES} tpie ${GDAL_LIBRARY})
