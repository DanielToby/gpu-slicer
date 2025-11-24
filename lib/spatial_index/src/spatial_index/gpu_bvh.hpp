#pragma once

#include <memory>
#include <vector>

#include "geometry/bbox.hpp"
#include "spatial_index/spatial_index.hpp"

namespace slicer {

//! Divides primitives into two distinct groups based on a heuristic until min number of primitives per leaf is achieved.
//! https://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
class GPUBVH : public I_SpatialIndex {
public:
    //! Constructs the spatial index.
    void build(const std::vector<Triangle3D>& triangles) override;

    //! Returns all triangles intersecting zPosition.
    [[nodiscard]] std::vector<Triangle3D> query(float zPosition) const override;

    //! The BBox of the index.
    [[nodiscard]] BBox3D AABB() const override;

    //! Returns the number of nodes in the BVH.
    [[nodiscard]] std::size_t getNumNodes() const;

    //! Returns the number of triangles in the BVH.
    [[nodiscard]] std::size_t getNumTriangles() const;

private:
};

}
