#pragma once

#include <algorithm>
#include <limits>

#include "slicer/geometry.hpp"
#include "slicer/intersect.hpp"

namespace slicer {

template <typename PointType>
struct BBoxHelpers {};

template <>
struct BBoxHelpers<Vec2> {
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
        return {std::min(lhs.x(), rhs.x()), std::min(lhs.y(), rhs.y())};
    }
    [[nodiscard]] static Vec2 combinedMax(const Vec2& lhs, const Vec2& rhs) {
        return {std::max(lhs.x(), rhs.x()), std::max(lhs.y(), rhs.y())};
    }
    [[nodiscard]] static float area(const Vec2& a, const Vec2& b) {
        auto spans = a.makeBinaryOp(b, [](float min, float max) { return max - min; });
        return spans.product();
    }
};

template <>
struct BBoxHelpers<QuantizedVec2> {
    [[nodiscard]] static QuantizedVec2 lowest() {
        return {std::numeric_limits<int64_t>::lowest(), std::numeric_limits<int64_t>::lowest()};
    }
    [[nodiscard]] static QuantizedVec2 highest() {
        return {std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::max()};
    }
    [[nodiscard]] static bool lessEqual(const QuantizedVec2& lhs, const QuantizedVec2& rhs) {
        return lhs.qx <= rhs.qx && lhs.qy <= rhs.qy;
    }
    [[nodiscard]] static QuantizedVec2 combinedMin(const QuantizedVec2& lhs, const QuantizedVec2& rhs) {
        return {std::min(lhs.qx, rhs.qx), std::min(lhs.qy, rhs.qy)};
    }
    [[nodiscard]] static QuantizedVec2 combinedMax(const QuantizedVec2& lhs, const QuantizedVec2& rhs) {
        return {std::max(lhs.qx, rhs.qx), std::max(lhs.qy, rhs.qy)};
    }
    [[nodiscard]] static int64_t area(const QuantizedVec2& a, const QuantizedVec2& b) {
        return (b.qx - a.qx) * (b.qy - a.qy);
    }
};

template <>
struct BBoxHelpers<Vec3> {
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
        return {std::min(lhs.x(), rhs.x()), std::min(lhs.y(), rhs.y()), std::min(lhs.z(), rhs.z())};
    }
    [[nodiscard]] static Vec3 combinedMax(const Vec3& lhs, const Vec3& rhs) {
        return {std::max(lhs.x(), rhs.x()), std::max(lhs.y(), rhs.y()), std::max(lhs.z(), rhs.z())};
    }
    [[nodiscard]] static float area(const Vec3& a, const Vec3& b) {
        auto spans = a.makeBinaryOp(b, [](float min, float max) { return max - min; });
        return spans.product();
    }
};

template <typename PointType>
struct BBox {
    PointType min{BBoxHelpers<PointType>::highest()};
    PointType max{BBoxHelpers<PointType>::lowest()};

    [[nodiscard]] bool empty() const {
        return BBoxHelpers<PointType>::lessEqual(max, min);
    }

    void extend(const PointType& vertex) {
        min = BBoxHelpers<PointType>::combinedMin(min, vertex);
        max = BBoxHelpers<PointType>::combinedMax(max, vertex);
    }

    void extend(const BBox& other) {
        this->extend(other.min);
        this->extend(other.max);
    }

    [[nodiscard]] BBox operator*(float s) const {
        return {{this->min * s}, {this->max * s}};
    }

    [[nodiscard]] typename PointType::ValueType area() const {
        return BBoxHelpers<PointType>::area(min, max);
    }

    [[nodiscard]] bool operator<(const BBox& other) const {
        return this->area() < other.area();
    }
};

using BBox2D = BBox<Vec2>;
using BBox3D = BBox<Vec3>;
using QuantizedBBox2D = BBox<QuantizedVec2>;

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