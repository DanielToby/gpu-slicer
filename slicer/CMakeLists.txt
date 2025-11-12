cmake_minimum_required(VERSION 3.29)

add_library(slicer STATIC)

set_target_properties(slicer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(slicer
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/stl_reader
)

set(SOURCES
    src/slicer/bbox.cpp
    src/slicer/bbox.hpp
    src/slicer/clipper.cpp
    src/slicer/clipper.hpp
    src/slicer/geometry.cpp
    src/slicer/geometry.hpp
    src/slicer/slicer.cpp
    src/slicer/slicer.hpp
    src/slicer/stl.cpp
    src/slicer/stl.hpp
    src/slicer/svg.cpp
    src/slicer/svg.hpp
    src/spatial_index/bvh.cpp
    src/spatial_index/bvh.hpp
    src/spatial_index/no_spatial_index.cpp
    src/spatial_index/no_spatial_index.hpp
    src/spatial_index/spatial_index.hpp
)

target_sources(slicer PUBLIC ${SOURCES})

# Tests

add_executable(slicer_tests
    src/slicer/bbox.test.cpp
    src/slicer/clipper.test.cpp
    src/slicer/slicer.test.cpp
    src/slicer/stl.test.cpp
    src/spatial_index/bvh.test.cpp
)

set_target_properties(slicer_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(
    slicer_tests
PRIVATE
    Catch2::Catch2WithMain
    slicer
)

include(CTest)
include(Catch)
catch_discover_tests(slicer_tests)
