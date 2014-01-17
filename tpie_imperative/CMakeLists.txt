find_package(Boost COMPONENTS date_time thread filesystem system regex)

include_directories(../tpie/tpie)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../tpie/tpie)

add_executable(tpie-imperative-transform transform.cpp ${COMMON_SOURCES})
target_link_libraries(tpie-imperative-transform ${Boost_LIBRARIES} tpie ${GDAL_LIBRARY})
