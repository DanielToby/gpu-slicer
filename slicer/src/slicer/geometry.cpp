#include "slicer/geometry.hpp"

namespace slicer {

std::size_t Vec2Hash::operator()(const Vec2& v) const noexcept {
    // Convert to integer grid coordinates
    const auto qx = static_cast<int64_t>(std::llround(v.x() / EPSILON));
    const auto qy = static_cast<int64_t>(std::llround(v.y() / EPSILON));

    // Combine using a strong integer hash
    std::size_t h = std::hash<int64_t>{}(qx);
    h ^= std::hash<int64_t>{}(qy) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);

    return h;
}

}