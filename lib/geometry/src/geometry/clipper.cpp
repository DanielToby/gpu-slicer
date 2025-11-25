#include "geometry/clipper.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>

namespace slicer {

namespace {

[[nodiscard]] bool insideInclusive(const Vec3& p, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return p.z >= zPosition;
    case KeepRegion::Below:
        return p.z <= zPosition;
    }
    throw std::invalid_argument("Invalid keep region");
}

[[nodiscard]] bool insideExclusive(const Vec3& p, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return p.z > zPosition;
    case KeepRegion::Below:
        return p.z < zPosition;
    }
    throw std::invalid_argument("Invalid keep region");
}

[[nodiscard]] std::optional<std::size_t> getFirstIndexWhere(const std::vector<Vec3>& vertices, const std::function<bool(const Vec3&)>& predicate) {
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        if (std::invoke(predicate, vertices[i])) {
            return i;
        }
    }
    return std::nullopt;
}

//! This checks for strictly above / below but NOT equal to ZPosition, so we can guarantee that there is complete geometry in keepRegion.
[[nodiscard]] std::optional<std::size_t> getStartingIndex(const std::vector<Vec3>& vertices, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
        case KeepRegion::Above:
            return getFirstIndexWhere(vertices, [&zPosition](const Vec3& vertex) { return vertex.z > zPosition; });
        case KeepRegion::Below:
            return getFirstIndexWhere(vertices, [&zPosition](const Vec3& vertex) { return vertex.z < zPosition; });
    }
    throw std::invalid_argument("Invalid keep region");
}

[[nodiscard]] bool allPointsInRegion(const std::vector<Vec3>& vertices, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return std::all_of(vertices.begin(), vertices.end(), [&zPosition](const Vec3& v) {
            return v.z >= zPosition;
        });
    case KeepRegion::Below:
        //! All points on ZPosition for KeepRegion::Below is considered out of bounds.
        if (std::all_of(vertices.begin(), vertices.end(), [&zPosition](const Vec3& v) {
                return v.z == zPosition;
            })) {
            return false;
        }
        return std::all_of(vertices.begin(), vertices.end(), [&zPosition](const Vec3& v) {
            return v.z <= zPosition;
        });
    }
    throw std::invalid_argument("Invalid keep region");
}

}

namespace detail {

LineBehavior lineBehavior(const Vec3& p0, const Vec3& p1, float zPosition, KeepRegion keepRegion) {
    if (insideInclusive(p0, zPosition, keepRegion) && insideInclusive(p1, zPosition, keepRegion)) {
        return LineBehavior::RemainsIn;
    }
    if (insideInclusive(p0, zPosition, keepRegion) && !insideInclusive(p1, zPosition, keepRegion)) {
        return LineBehavior::Exits;
    }
    if (!insideExclusive(p0, zPosition, keepRegion) && !insideExclusive(p1, zPosition, keepRegion)) {
        return LineBehavior::RemainsOut;
    }
    if (!insideInclusive(p0, zPosition, keepRegion) && insideExclusive(p1, zPosition, keepRegion)) {
        return LineBehavior::Enters;
    }
    throw std::runtime_error("Unhandled line behavior.");
}

}

Polygon3D clip(const Polygon3D& polygon, float zPosition, KeepRegion keepRegion) {
    if (!polygon.isValid()) {
        throw std::invalid_argument("Invalid polygon.");
    }

    if (allPointsInRegion(polygon.vertices, zPosition, keepRegion)) {
        return polygon;
    }

    auto result = Polygon3D{};
    auto offset = getStartingIndex(polygon.vertices, zPosition, keepRegion);
    if (!offset) {
        // No geometry in region.
        return {};
    }

    for (auto i = 0; i < polygon.vertices.size(); ++i) {
        const auto p0 = polygon.vertices[(i + *offset) % polygon.vertices.size()];
        const auto p1 = polygon.vertices[(i + *offset + 1) % polygon.vertices.size()];

        using detail::LineBehavior;
        switch (detail::lineBehavior(p0, p1, zPosition, keepRegion)) {
        case LineBehavior::RemainsIn: {
            result.vertices.push_back(p0);
            break;
        }
        case LineBehavior::Exits: {
            result.vertices.push_back(p0);
            if (p0.z != zPosition) {
                // Special case: p0 on Z -> p1 out is considered an exit. Don't double-count p0.
                const auto intersection = intersect(Segment3D{p0, p1}, zPosition);
                if (!intersection) {
                    throw std::invalid_argument("Invalid intersection.");
                }
                result.vertices.push_back(*intersection);
            }
            break;
        }
        case LineBehavior::Enters: {
            const auto intersection = intersect(Segment3D{p0, p1}, zPosition);
            if (!intersection) {
                throw std::invalid_argument("Invalid intersection.");
            }
            result.vertices.push_back(*intersection);
            break;
        }
        case LineBehavior::RemainsOut:
            break;
        }
    }
    return result;
}

}