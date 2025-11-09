cmake_minimum_required(VERSION 3.29)

add_library(slicer STATIC)

set_target_properties(slicer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(slicer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
    src/slicer/bvh.cpp
    src/slicer/bvh.hpp
)

target_sources(slicer PUBLIC ${SOURCES})
