#pragma once

#include <array>

namespace slicer {

using Vec3 = std::array<float, 3>;
struct Triangle {
    Vec3 normal;
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
};

}