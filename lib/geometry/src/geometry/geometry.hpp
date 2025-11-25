#pragma once

#include <array>
#include <ostream>
#include <numeric>
#include <vector>

namespace slicer {

namespace detail {

template <typename Vec, typename Op, std::size_t... I>
[[nodiscard]] constexpr Vec makeUnaryOpImpl(const Vec& a, Op op, std::index_sequence<I...>) {
    return Vec{op(a.data()[I])...};
}

template <typename Vec, typename Op, std::size_t... I>
[[nodiscard]] constexpr Vec makeBinaryOpImpl(const Vec& a, const Vec& b, Op op, std::index_sequence<I...>) {
    return Vec{op(a.data()[I], b.data()[I])...};
}

}

template <typename Vec, typename Op>
constexpr Vec makeUnaryOp(const Vec& a, Op op) {
    return detail::makeUnaryOpImpl(a, op, std::make_index_sequence<Vec::dim>{});
}

template <typename Vec, typename Op>
constexpr Vec makeBinaryOp(const Vec& a, const Vec& b, Op op) {
    return detail::makeBinaryOpImpl(a, b, op, std::make_index_sequence<Vec::dim>{});
}

struct Vec2 {
    using ValueType = float;
    static constexpr std::size_t dim = 2;

    ValueType x;
    ValueType y;

    [[nodiscard]] std::array<float, 2> data() const { return {x, y}; }

    [[nodiscard]] bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const {
        return makeBinaryOp(*this, other, [](float av, float bv) { return av + bv; });
    }

    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const {
        return makeBinaryOp(*this, other, [](float av, float bv) { return av - bv; });
    }

    [[nodiscard]] constexpr Vec2 operator*(const Vec2& other) const {
        return makeBinaryOp(*this, other, [](float av, float bv) { return av * bv; });
    }

    [[nodiscard]] constexpr Vec2 operator*(float s) const {
        return makeUnaryOp(*this, [&s](float a) { return a * s; });
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

    [[nodiscard]] std::array<float, 3> data() const { return {x, y, z}; }

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const {
        return makeBinaryOp(*this, other, [](float av, float bv) { return av + bv; });
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const {
        return makeBinaryOp(*this, other, [](float av, float bv) { return av - bv; });
    }

    [[nodiscard]] constexpr Vec3 operator*(const Vec3& other) const {
        return makeBinaryOp(*this, other, [](float av, float bv) { return av * bv; });
    }

    [[nodiscard]] constexpr Vec3 operator*(float s) const {
        return makeUnaryOp(*this, [&s](float a) { return a * s; });
    }

    [[nodiscard]] Vec2 toVec2() const { return {x, y}; }
};

template <typename Vec>
[[nodiscard]] constexpr float sum(const Vec& a) {
    return std::accumulate(a.data().begin(), a.data().end(), 0.0f);
}

template <typename Vec>
[[nodiscard]] constexpr float product(const Vec& a) {
    return std::accumulate(a.data().begin(), a.data().end(), 1.0f,
        [](float a, float b) { return a * b; });
}

template <typename Vec>
[[nodiscard]] static constexpr float dot(const Vec& a, const Vec& b) {
    return sum(a * b);
}

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