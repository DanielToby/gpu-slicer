#include "slicer/slicer.hpp"

#include "clipper.hpp"

#include <ranges>

#include <iostream>

namespace slicer {

namespace {

[[nodiscard]] Polygon3D toPolygon3D(const Triangle3D& triangle) {
    return {{triangle.v0, triangle.v1, triangle.v2},};
}

[[nodiscard]] Polygon2D toPolygon2D(const Polygon3D& polygon) {
    auto vertices = polygon.vertices | std::views::transform(&toVec2);
    return {{vertices.begin(), vertices.end()}};
}

}

std::vector<Span> getSliceHeights(const BBox3D& volume, float thickness_mm) {
    std::vector<Span> result;
    const auto bottom = volume.min.z;

    auto current = bottom;
    do {
        auto next = current + thickness_mm;
        result.emplace_back(current, std::min(next, volume.max.z));
        current = next;
    } while (current < volume.max.z);

    return result;
}

std::vector<Slice> slice(const I_SpatialIndex& mesh, float thickness_mm) {
    std::vector<Slice> result;

    for (const auto& sliceHeight : getSliceHeights(mesh.AABB(), thickness_mm)) {
        auto slicePolygons =
            mesh.query(sliceHeight) |
            std::views::transform([&sliceHeight](const auto& triangle) {
                const auto withClippedBottom = clip(toPolygon3D(triangle), sliceHeight.lower(), KeepRegion::Above);

                if (withClippedBottom.isEmpty()) {
                    // TODO: Fix!
                    return Polygon2D{};
                }

                const auto withClippedTop = clip(withClippedBottom, sliceHeight.upper(), KeepRegion::Below);
                return toPolygon2D(withClippedTop);
            });

        result.push_back({{slicePolygons.begin(), slicePolygons.end()}, sliceHeight});
    }

    return result;
}

}
