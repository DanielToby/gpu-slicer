#pragma once

#include <limits>

#include "slicer/geometry.hpp"

#include <algorithm>

namespace slicer {

struct BBox2D {
    Vec2 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec2 max{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

    [[nodiscard]] bool empty() const {
        return min.x >= max.x && min.y >= max.y;
    }

    void extend(const Vec2& vertex) {
        min.x = std::min(min.x, vertex.x);
        min.y = std::min(min.y, vertex.y);

        max.x = std::max(max.x, vertex.x);
        max.y = std::max(max.y, vertex.y);
    }
};

struct BBox3D {
    Vec3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec3 max{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

    [[nodiscard]] bool empty() const {
        return min.x >= max.x && min.y >= max.y && min.z >= max.z;
    }

    void extend(const Vec3& vertex) {
        min.x = std::min(min.x, vertex.x);
        min.y = std::min(min.y, vertex.y);
        min.z = std::min(min.z, vertex.z);

        max.x = std::max(max.x, vertex.x);
        max.y = std::max(max.y, vertex.y);
        max.z = std::max(max.z, vertex.z);
    }
};

[[nodiscard]] BBox2D getAABB(const Polygon2D& polygon);

[[nodiscard]] BBox3D getAABB(const Polygon3D& polygon);

}