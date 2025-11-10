#pragma once

#include <ranges>
#include <vector>

namespace slicer {

struct Vec2 {
    float x;
    float y;

    [[nodiscard]] bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }
};

struct Vec3 {
    float x;
    float y;
    float z;

    [[nodiscard]] bool operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }
};

[[nodiscard]] inline Vec2 toVec2(const Vec3& vec3) {
    return {vec3.x, vec3.y};
}

template <typename T>
struct Polygon {
    std::vector<T> vertices;

    template <typename Tx>
    [[nodiscard]] Polygon transform(Tx&& tx) const {
        auto polys = vertices | std::views::transform(tx);
        return {{polys.begin(), polys.end()}};
    }

    [[nodiscard]] Polygon scale(float scalar) const {
        return this->transform([&scalar](const auto& v) { return v * scalar; });
    }

    [[nodiscard]] Polygon translate(const T& amount) const {
        return this->transform([&amount](const auto& v) { return v + amount; });
    }

    [[nodiscard]] bool operator==(const Polygon& other) const { return vertices == other.vertices; }
    [[nodiscard]] bool isEmpty() const { return vertices.empty(); }
    [[nodiscard]] bool isValid() const { return vertices.size() > 2; }
};

using Polygon2D = Polygon<Vec2>;
using Polygon3D = Polygon<Vec3>;

struct Triangle2D {
    Vec2 v0;
    Vec2 v1;
    Vec2 v2;
};

struct Triangle3D {
    Vec3 normal;
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
};

template <typename T>
struct Line {
    T p0;
    T direction;

    [[nodiscard]] static Line fromPoints(const T& p0, const T& p1) {
        return {p0, p1 - p0};
    }
};

using Line2D = Line<Vec2>;
using Line3D = Line<Vec3>;

struct Plane {
    Vec3 p0;
    Vec3 normal;
};

}