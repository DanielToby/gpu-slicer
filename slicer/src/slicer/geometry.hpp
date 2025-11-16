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

    [[nodiscard]] constexpr Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    [[nodiscard]] static constexpr float dot(const Vec2& a, const Vec2& b) {
        return a.x * b.x + a.y * b.y;
    }

    [[nodiscard]] static constexpr float cross(const Vec2& a, const Vec2& b) {
        return a.x * b.x - a.y * b.y;
    }
};

struct Vec2Hash {
    std::size_t operator()(const Vec2& v) const noexcept;
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

    [[nodiscard]] static constexpr float dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};

[[nodiscard]] inline Vec2 toVec2(const Vec3& vec3) {
    return {vec3.x, vec3.y};
}

template <typename T>
struct Segment {
    T v0;
    T v1;
};

using Segment2D = Segment<Vec2>;
using Segment3D = Segment<Vec3>;

template <typename T>
struct Polygon {
    std::vector<T> vertices;
    std::vector<Polygon> holes;

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

//! Warning! This library performs quantization as soon as intersections are calculated.
//! This ensures a snapping of values and correct comparison / hashing at later steps.
//! It'd probably be more correct to perform quantization at every step, but this is just a demo.
class QuantizedLine2D {
public:
    QuantizedLine2D(const Vec2& v0, const Vec2& v1) : m_v0(quantize(v0)), m_v1(quantize(v1)) {}

    [[nodiscard]] Vec2 v0() const { return m_v0; }
    [[nodiscard]] Vec2 v1() const { return m_v1; }

private:
    [[nodiscard]] static Vec2 quantize(const Vec2& original);

    Vec2 m_v0;
    Vec2 m_v1;
};

//! Intersects the triangle with the plane described by zPosition. Returns a 2D line segment.
//! this function's implementation describes the rules by which we keep intersections with triangles lying directly on the z-position.
[[nodiscard]] std::optional<QuantizedLine2D> intersect(const Triangle3D& triangle, float zPosition);

//! Returns all segments created by intersecting the provided triangles.
[[nodiscard]] std::vector<QuantizedLine2D> intersect(std::span<const Triangle3D> triangles, float zPosition);

[[nodiscard]] bool intersects(const Segment3D& segment, float zPosition);

[[nodiscard]] std::optional<Vec3> intersect(const Segment3D& segment, float zPosition);

[[nodiscard]] std::optional<Vec2> intersect(const Segment2D& segment, const Ray2D& line);

}