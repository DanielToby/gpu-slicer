#pragma once

#include "spatial_index/spatial_index.hpp"

namespace slicer {

class NoSpatialIndex : public I_SpatialIndex {
public:
    NoSpatialIndex() = default;
    void build(const std::vector<Triangle3D>& triangles) override;
    [[nodiscard]] std::vector<Triangle3D> query(float inRange) const override;
    [[nodiscard]] BBox3D AABB() const override;

private:
    std::vector<Triangle3D> m_triangles;
};

}