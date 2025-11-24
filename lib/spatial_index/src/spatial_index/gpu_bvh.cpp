#include "gpu_bvh.hpp"

#include <stdexcept>

#include "config.hpp"

#include "spatial_index/bvh.hpp"

#if HAVE_CUDA
#include "cuda/kernels/intersect_kernel.cuh"
#endif

namespace slicer {

namespace {

[[nodiscard]] BBox3D getAABB(const detail::BVHLeaf& leaf) {
    BBox3D result;
    for (const auto& triangle : leaf.primitives) {
        result.extend(getAABB(triangle));
    }
    return result;
}

[[nodiscard]] cuda::kernels::GPUVec3 toGPUVec3(const Vec3& vec3) {
    return {vec3.x(), vec3.y(), vec3.z()};
}

[[nodiscard]] Vec3 fromGPUVec3(const cuda::kernels::GPUVec3& gpuVec3) {
    return {gpuVec3.x, gpuVec3.y, gpuVec3.z};
}

[[nodiscard]] cuda::kernels::GPUTriangle3D toGPUTriangle3D(const Triangle3D& triangle) {
    return {toGPUVec3(triangle.v0), toGPUVec3(triangle.v1), toGPUVec3(triangle.v2)};
}

[[nodiscard]] Triangle3D fromGPUTriangle3D(const cuda::kernels::GPUTriangle3D& gpuTriangle3D) {
    return {fromGPUVec3(gpuTriangle3D.v0), fromGPUVec3(gpuTriangle3D.v1), fromGPUVec3(gpuTriangle3D.v2)};
}

}

void GPUBVH::build(const std::vector<Triangle3D>& triangles) {
    auto zSortedTriangles = detail::getZSortedBVHTriangles(triangles);
    auto root = detail::getBVHNode(zSortedTriangles, ConstructionStrategy::BinarySplit);

}

std::vector<Triangle3D> GPUBVH::query(float zPosition) const {
#if HAVE_CUDA
    // runAddKernel(a, b, out, 4);
#endif

    throw std::runtime_error("Not implemented.");
}

BBox3D GPUBVH::AABB() const {
    throw std::runtime_error("Not implemented.");
}

std::size_t GPUBVH::getNumNodes() const {
    throw std::runtime_error("Not implemented.");
}

std::size_t GPUBVH::getNumTriangles() const {
    throw std::runtime_error("Not implemented.");
}

}
