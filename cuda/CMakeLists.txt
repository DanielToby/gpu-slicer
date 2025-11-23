add_library(slicer_cuda STATIC
    src/cuda/kernels/add_kernel.cu
)

set_target_properties(slicer_cuda PROPERTIES
    CUDA_STANDARD 17
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_ARCHITECTURES 89  # RTX 4070
)

target_include_directories(slicer_cuda PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
