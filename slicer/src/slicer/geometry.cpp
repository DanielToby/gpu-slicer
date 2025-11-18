#include "slicer/geometry.hpp"

namespace slicer {

constexpr float EPSILON = 1e-6f;

void addToHash(std::size_t& hash, const int64_t toAdd) {
    hash ^= std::hash<int64_t>{}(toAdd) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
}

std::size_t hash(const std::vector<int64_t>& inputs) {
    std::size_t result = 0x9e3779b97f4a7c15ull;
    for (const auto& input : inputs) {
        addToHash(result, input);
    }
    return result;
}

std::size_t Vec2Hash::operator()(const Vec2& v) const noexcept {
    // Convert to integer grid coordinates
    const auto qx = static_cast<int64_t>(std::llround(v.x() / EPSILON));
    const auto qy = static_cast<int64_t>(std::llround(v.y() / EPSILON));
    return hash({qx, qy});
}

std::size_t BidirectionalSegment2DHash::operator()(const Segment2D& segment) const noexcept {
    Vec2Hash pointHash;
    auto h0 = static_cast<int64_t>(pointHash(segment.v0));
    auto h1 = static_cast<int64_t>(pointHash(segment.v1));

    // Enforce direction-agnostic order
    if (h1 < h0) {
        std::swap(h0, h1);
    }
    return hash({h0, h1});
}

}