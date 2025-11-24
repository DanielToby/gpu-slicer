#include "bvh.hpp"

#include <catch2/catch_test_macros.hpp>

#include "spatial_index/gpu_bvh.hpp"

using namespace slicer;

namespace {
[[nodiscard]] Vec3 makePoint(float v) {
    return {v, v, v};
}

std::vector<Triangle3D> makeTriangles(std::size_t numTriangles) {
    std::vector<Triangle3D> triangles;
    auto v = 0.f;
    for (std::size_t i = 0; i < numTriangles; i++) {
        triangles.push_back(Triangle3D{makePoint(++v), makePoint(++v), makePoint(++v)});
    }
    return triangles;
}

}

TEST_CASE("GPU BVH: construct") {
    auto triangles = makeTriangles(10);

    auto bvh = BVH(ConstructionStrategy::BinarySplit);
    bvh.build(triangles);

    // auto gpuBvh = GPUBVH();
    // gpuBvh.build(triangles);
    //
    // CHECK(gpuBvh.getNumNodes() == bvh.getNumNodes());
    // CHECK(gpuBvh.getNumTriangles() == triangles.size());
}