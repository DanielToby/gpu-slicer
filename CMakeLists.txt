cmake_minimum_required(VERSION 3.29)
project(slicer LANGUAGES CXX)

option(ENABLE_CUDA "Build with CUDA acceleration" ON)

add_subdirectory(vendor/catch2)

if (ENABLE_CUDA)
    enable_language(CUDA OPTIONAL)
    if (CMAKE_CUDA_COMPILER)
        message(STATUS "CUDA enabled: ${CMAKE_CUDA_COMPILER}")
        add_subdirectory(cuda)
        set(HAVE_CUDA 1 CACHE INTERNAL "CUDA availability")
    else()
        set(HAVE_CUDA 0 CACHE INTERNAL "CUDA availability")
    endif()
endif()

add_subdirectory(slicer)
