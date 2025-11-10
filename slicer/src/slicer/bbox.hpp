#pragma once

#include <limits>

#include "slicer/fwd.hpp"

namespace slicer {

struct BBox2D {
    Vec2 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec2 max{std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

    [[nodiscard]] bool empty() const {
        return min.x >= max.x && min.y >= max.y;
    }
};

struct BBox3D {
    Vec3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec3 max{std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

    [[nodiscard]] bool empty() const {
        return min.x >= max.x && min.y >= max.y && min.z >= max.z;
    }
};

}