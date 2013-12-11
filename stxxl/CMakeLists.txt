add_subdirectory(stxxl)
include_directories(stxxl/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/stxxl/include)

add_executable(stxxl-transform transform.cpp ${COMMON_SOURCES})
target_link_libraries(stxxl-transform ${STXXL_LIBRARIES} ${GDAL_LIBRARY})
