#pragma once

#include <limits>

#include "slicer/geometry.hpp"

#include <algorithm>

namespace slicer {

template <typename PointType>
struct PointTypeLimits {};

template <>
struct PointTypeLimits<Vec2> {
    [[nodiscard]] static Vec2 lowest() {
        return {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    }
    [[nodiscard]] static Vec2 highest() {
        return {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    }
    [[nodiscard]] static bool lessEqual(const Vec2& lhs, const Vec2& rhs) {
        return lhs.x() <= rhs.x() && lhs.y() <= rhs.y();
    }
    [[nodiscard]] static Vec2 combinedMin(const Vec2& lhs, const Vec2& rhs) {
        return {std::min(lhs.x(), lhs.x()), std::min(rhs.y(), rhs.y())};
    }
    [[nodiscard]] static Vec2 combinedMax(const Vec2& lhs, const Vec2& rhs) {
        return {std::max(lhs.x(), lhs.x()), std::max(rhs.y(), rhs.y())};
    }

};

template <>
struct PointTypeLimits<Vec3> {
    [[nodiscard]] static Vec3 lowest() {
        return {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    }
    [[nodiscard]] static Vec3 highest() {
        return {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    }
    [[nodiscard]] static bool lessEqual(const Vec3& lhs, const Vec3& rhs) {
        return lhs.x() <= rhs.x() && lhs.y() <= rhs.y() && lhs.z() <= rhs.z();
    }
    [[nodiscard]] static Vec3 combinedMin(const Vec3& lhs, const Vec3& rhs) {
        return {std::min(lhs.x(), lhs.x()), std::min(rhs.y(), rhs.y()), std::min(lhs.z(), rhs.z())};
    }
    [[nodiscard]] static Vec3 combinedMax(const Vec3& lhs, const Vec3& rhs) {
        return {std::max(lhs.x(), lhs.x()), std::max(rhs.y(), rhs.y()), std::max(lhs.z(), rhs.z())};
    }
};

template <typename PointType>
struct BBox {
    PointType min{PointTypeLimits<PointType>::highest()};
    PointType max{PointTypeLimits<PointType>::lowest()};

    [[nodiscard]] bool empty() const {
        return PointTypeLimits<PointType>::lessEqual(max, min);
    }

    void extend(const PointType& vertex) {
        min = PointTypeLimits<PointType>::combinedMin(min, vertex);
        max = PointTypeLimits<PointType>::combinedMax(min, vertex);
    }

    void extend(const BBox& other) {
        this->extend(other.min);
        this->extend(other.max);
    }

    [[nodiscard]] BBox operator*(float s) const {
        return {{this->min * s}, {this->max * s}};
    }

    [[nodiscard]] float area() const {
        auto spans = min.makeBinaryOp(max, [](float min, float max) { return max - min; });
        return spans.product();
    }

    [[nodiscard]] bool operator<(const BBox& other) const {
        return this->area() < other.area();
    }
};

using BBox2D = BBox<Vec2>;
using BBox3D = BBox<Vec3>;

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