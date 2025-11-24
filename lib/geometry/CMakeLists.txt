cmake_minimum_required(VERSION 3.29)

add_library(geometry STATIC)

set_target_properties(geometry PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(geometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCES
    src/geometry/bbox.cpp
    src/geometry/bbox.hpp
    src/geometry/clipper.cpp
    src/geometry/clipper.hpp
    src/geometry/geometry.cpp
    src/geometry/geometry.hpp
    src/geometry/intersect.cpp
    src/geometry/intersect.hpp
)

target_sources(geometry PUBLIC ${SOURCES})

# Tests

add_executable(geometry_tests
    src/geometry/bbox.test.cpp
    src/geometry/clipper.test.cpp
    src/geometry/intersect.test.cpp
)

set_target_properties(geometry_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(
    geometry_tests
PRIVATE
    Catch2::Catch2WithMain
    geometry
    spatial_index
)

include(CTest)
include(Catch)
catch_discover_tests(geometry_tests)
