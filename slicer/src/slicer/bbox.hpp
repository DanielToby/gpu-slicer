#pragma once

#include <limits>

#include "slicer/geometry.hpp"

#include <algorithm>

namespace slicer {

struct BBox2D {
    Vec2 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec2 max{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

    [[nodiscard]] bool empty() const {
        return min.x() >= max.x() && min.y() >= max.y();
    }

    void extend(const Vec2& vertex) {
        min = Vec2{std::min(min.x(), vertex.x()), std::min(min.y(), vertex.y())};
        max = Vec2{std::max(max.x(), vertex.x()), std::max(max.y(), vertex.y())};
    }

    void extend(const BBox2D& other) {
        this->extend(other.min);
        this->extend(other.max);
    }

    [[nodiscard]] BBox2D operator*(float s) const {
        return {Vec2{this->min * s}, Vec2{this->max * s}};
    }

    [[nodiscard]] float area() const {
        return (max.x() - min.x()) * (max.y() - min.y());
    }

    [[nodiscard]] bool operator<(const BBox2D& other) const {
        return this->area() < other.area();
    }
};

struct BBox3D {
    Vec3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec3 max{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

    [[nodiscard]] bool empty() const {
        return min.x() >= max.x() && min.y() >= max.y() && min.z() >= max.z();
    }

    void extend(const Vec3& vertex) {
        min = Vec3{std::min(min.x(), vertex.x()), std::min(min.y(), vertex.y()), std::min(min.z(), vertex.z())};
        max = Vec3{std::max(max.x(), vertex.x()), std::max(max.y(), vertex.y()), std::max(max.z(), vertex.z())};
    }

    void extend(const BBox3D& other) {
        this->extend(other.min);
        this->extend(other.max);
    }
};

[[nodiscard]] inline BBox2D toBBox2D(const BBox3D& bbox) {
    return {bbox.min.as<Vec2>(), bbox.max.as<Vec2>() };
}

[[nodiscard]] BBox2D getAABB(std::span<const Vec2> vertices);

[[nodiscard]] BBox3D getAABB(std::span<const Vec3> vertices);

[[nodiscard]] BBox2D getAABB(const Polygon2D& polygon);

[[nodiscard]] BBox3D getAABB(const Polygon3D& polygon);

[[nodiscard]] BBox2D getAABB(const Triangle2D& triangle);

[[nodiscard]] BBox3D getAABB(const Triangle3D& triangle);

}