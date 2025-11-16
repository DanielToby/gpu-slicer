#pragma once

#include "slicer/bbox.hpp"
#include "slicer/geometry.hpp"

namespace slicer {

//! This is the core type of this project. The goal is to create several of these, and time the impact that
//! each one has on slice duration.

class I_SpatialIndex {
public:
    virtual ~I_SpatialIndex() = default;

    //! Constructs the spacial index.
    virtual void build(const std::vector<Triangle3D>& triangles) = 0;

    //! Returns all triangles intersecting zPosition.
    [[nodiscard]] virtual std::vector<Triangle3D> query(float zPosition) const = 0;

    //! The BBox of the index.
    [[nodiscard]] virtual BBox3D AABB() const = 0;
};

}