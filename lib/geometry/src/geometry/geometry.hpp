#pragma once

#include <array>
#include <ostream>
#include <numeric>
#include <vector>

namespace slicer {

struct Vec2 {
    using ValueType = float;
    static constexpr std::size_t dim = 2;

    ValueType x;
    ValueType y;

    [[nodiscard]] bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vec2 operator*(const Vec2& other) const {
        return {x * other.x, y * other.y};
    }

    [[nodiscard]] constexpr Vec2 operator*(float s) const {
        return {x * s, y * s};
    }

    [[nodiscard]] constexpr float sum() const {
        return x + y;
    }

    [[nodiscard]] constexpr float product() const {
        return x * y;
    }

    [[nodiscard]] static constexpr float dot(const Vec2& a, const Vec2& b) {
        return (a * b).sum();
    }

};

struct Vec3 {
    using ValueType = float;
    static constexpr std::size_t dim = 3;

    ValueType x;
    ValueType y;
    ValueType z;

    [[nodiscard]] bool operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3 operator*(const Vec3& other) const {
        return {x * other.x, y * other.y, z * other.z};
    }

    [[nodiscard]] constexpr Vec3 operator*(float s) const {
        return {x * s, y * s, z * s};
    }

    [[nodiscard]] constexpr float sum() const {
        return x + y + z;
    }

    [[nodiscard]] constexpr float product() const {
        return x * y * z;
    }

    [[nodiscard]] static constexpr float dot(const Vec3& a, const Vec3& b) {
        return (a * b).sum();
    }

    [[nodiscard]] Vec2 toVec2() const { return {x, y}; }
};

template <typename PointType>
struct Segment {
    PointType v0;
    PointType v1;

    [[nodiscard]] bool operator==(const Segment& other) const {
        return v0 == other.v0 && v1 == other.v1;
    }
};

using Segment2D = Segment<Vec2>;
using Segment3D = Segment<Vec3>;

template <typename PointType>
struct Polygon {
    std::vector<PointType> vertices;
    std::vector<Polygon> holes;

    template <typename Tx>
    [[nodiscard]] Polygon transform(const Tx& tx) const {
        auto result = *this;
        for (auto& v : result.vertices) {
            std::invoke(tx, v);
        }
        for (auto& h : result.holes) {
            transform(tx, h);
        }
        return result;
    }

    [[nodiscard]] Polygon scale(float scalar) const {
        return this->transform([&scalar](const auto& v) { return v * scalar; });
    }

    [[nodiscard]] Polygon translate(const PointType& amount) const {
        return this->transform([&amount](const auto& v) { return v + amount; });
    }

    [[nodiscard]] bool operator==(const Polygon& other) const { return vertices == other.vertices; }
    [[nodiscard]] bool isEmpty() const { return vertices.empty(); }
    [[nodiscard]] bool isValid() const { return vertices.size() > 2; }
};

using Polygon2D = Polygon<Vec2>;
using Polygon3D = Polygon<Vec3>;

template <typename PointType>
struct Triangle {
    PointType v0;
    PointType v1;
    PointType v2;
};

using Triangle2D = Triangle<Vec2>;
using Triangle3D = Triangle<Vec3>;

template <typename T>
struct Ray {
    T p0;
    T direction;

    [[nodiscard]] static Ray fromPoints(const T& p0, const T& p1) {
        return {p0, p1 - p0};
    }
};

using Ray2D = Ray<Vec2>;
using Ray3D = Ray<Vec3>;

struct Plane {
    Vec3 p0;
    Vec3 normal;
};

std::ostream& operator<<(std::ostream& os, const Vec2& v);
std::ostream& operator<<(std::ostream& os, const Vec3& v);
std::ostream& operator<<(std::ostream& os, const Segment2D& v);
std::ostream& operator<<(std::ostream& os, const Segment3D& v);
std::ostream& operator<<(std::ostream& os, const Triangle3D& v);

}