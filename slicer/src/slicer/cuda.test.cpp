#include <catch2/catch_test_macros.hpp>

#include "config.hpp"

#if HAVE_CUDA
#include "cuda/kernels/add_kernel.cuh"
#endif

TEST_CASE("CUDA: addKernel") {
#if HAVE_CUDA
    int a[4] = {1,2,3,4};
    int b[4] = {10,20,30,40};
    int out[4];

    runAddKernel(a, b, out, 4);

    REQUIRE(out[0] == 11);
    REQUIRE(out[1] == 22);
    REQUIRE(out[2] == 33);
    REQUIRE(out[3] == 44);
#else
    WARN("CUDA not available, skipping addKernel test");
#endif
}
