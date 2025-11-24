cmake_minimum_required(VERSION 3.29)

add_library(spatial_index STATIC)

set_target_properties(spatial_index PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(spatial_index
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR} # For config.hpp
    $<$<BOOL:${HAVE_CUDA}>:${CMAKE_SOURCE_DIR}/cuda/src>
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
)

set(SOURCES
    src/spatial_index/bvh.cpp
    src/spatial_index/bvh.hpp
    src/spatial_index/gpu_bvh.cpp
    src/spatial_index/gpu_bvh.hpp
    src/spatial_index/no_spatial_index.cpp
    src/spatial_index/no_spatial_index.hpp
    src/spatial_index/spatial_index.hpp
)

target_sources(spatial_index PUBLIC ${SOURCES})

target_link_libraries(
    spatial_index
PUBLIC
    geometry
)

if (TARGET slicer_cuda)
    target_link_libraries(spatial_index PRIVATE slicer_cuda)
endif()

# Tests

add_executable(spatial_index_tests
    src/spatial_index/bvh.test.cpp
    src/spatial_index/gpu_bvh.test.cpp
    src/spatial_index/variant.hpp
)

set_target_properties(spatial_index_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(
    spatial_index_tests
PRIVATE
    Catch2::Catch2WithMain
    spatial_index
)

if(TARGET slicer_cuda)
    target_link_libraries(spatial_index_tests PRIVATE slicer_cuda)
endif()

include(CTest)
include(Catch)
catch_discover_tests(spatial_index_tests)
