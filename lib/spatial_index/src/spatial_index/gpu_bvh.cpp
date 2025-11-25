#include "gpu_bvh.hpp"

#include <stdexcept>

#include "config.hpp"

#include "spatial_index/bvh.hpp"

#if HAVE_CUDA
#include "cuda/kernels/intersect_kernel.cuh"
#endif

namespace slicer {

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
