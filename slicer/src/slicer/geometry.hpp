#pragma once

#include <array>
#include <numeric>
#include <ranges>
#include <vector>

namespace slicer {

//! Epsilon is used for hashing and quantization of points.
constexpr float EPSILON = 1e-6f;

//! All members common to Vectors of N dimensions.
template<std::size_t N>
class VecBase {
public:
    VecBase() = default;

    template <typename... Ts>
    explicit VecBase(Ts&&... xs) :
        data{{static_cast<float>(xs)...}} {
        static_assert(sizeof...(Ts) == N);
    }

    [[nodiscard]] bool operator==(const VecBase& other) const = default;

    [[nodiscard]] constexpr VecBase operator+(const VecBase& other) const {
        return makeBinaryOp(other, [](float a, float b) { return a + b; });
    }

    [[nodiscard]] constexpr VecBase operator-(const VecBase& other) const {
        return makeBinaryOp(other, [](float a, float b) { return a - b; });
    }

    [[nodiscard]] constexpr VecBase operator*(const VecBase& other) const {
        return makeBinaryOp(other, [](float a, float b) { return a * b; });
    }

    [[nodiscard]] constexpr VecBase operator*(float s) const {
        return makeUnaryOp([&s](float a) { return a * s; });
    }

    [[nodiscard]] constexpr float sum() const {
        return std::accumulate(data.begin(), data.end(), 0.0f);
    }

    [[nodiscard]] static constexpr float dot(const VecBase& a, const VecBase& b) {
        return (a * b).sum();
    }

protected:
    std::array<float, N> data{};

private:
    template <typename Op, std::size_t... I>
        constexpr VecBase makeUnaryOpImpl(Op op, std::index_sequence<I...>) const {
        return VecBase{op(data[I])...};
    }

    template <typename Op>
    constexpr VecBase makeUnaryOp(Op op) const {
        return makeUnaryOpImpl(op, std::make_index_sequence<N>{});
    }

    template <typename Op, std::size_t... I>
    constexpr VecBase makeBinaryOpImpl(const VecBase& o, Op op, std::index_sequence<I...>) const {
        return VecBase{op(data[I], o.data[I])...};
    }

    template <typename Op>
    constexpr VecBase makeBinaryOp(const VecBase& o, Op op) const {
        return makeBinaryOpImpl(o, op, std::make_index_sequence<N>{});
    }
};

template<std::size_t N>
class Vec : public VecBase<N> {
public:
    using VecBase<N>::VecBase;
protected:
    using VecBase<N>::data;
};

//! Specializations on this type allow explicit valid conversions between vector types.
template <typename From, typename To>
struct VecConvert {};

template <>
class Vec<2> : public VecBase<2> {
public:
    Vec() = default;
    Vec(float x, float y) : VecBase(x, y) {}
    constexpr Vec(const VecBase& base) : VecBase(base) {}

    [[nodiscard]] float x() const { return this->data[0]; }
    [[nodiscard]] float y() const { return this->data[1]; }

    template <typename To>
    [[nodiscard]] To as() const {
        return VecConvert<Vec<2>, To>::to(*this);
    }
};
using Vec2 = Vec<2>;

template <>
class Vec<3> : public VecBase<3> {
public:
    Vec() = default;
    Vec(float x, float y, float z) : VecBase(x, y, z) {}
    constexpr Vec(const VecBase& base) : VecBase(base) {}

    [[nodiscard]] float x() const { return this->data[0]; }
    [[nodiscard]] float y() const { return this->data[1]; }
    [[nodiscard]] float z() const { return this->data[2]; }

    template <typename To>
    [[nodiscard]] To as() const {
        return VecConvert<Vec<3>, To>::to(*this);
    }
};
using Vec3 = Vec<3>;

//! This is the only supported vector conversion.
template<>
struct VecConvert<Vec3, Vec2> {
    [[nodiscard]] static Vec2 to(const Vec3& v) {
        return {v.x(), v.y()};
    }
};

struct Vec2Hash {
    std::size_t operator()(const Vec2& v) const noexcept;
};

template <typename PointType>
struct Segment {
    PointType v0;
    PointType v1;
};

using Segment2D = Segment<Vec2>;
using Segment3D = Segment<Vec3>;

template <typename PointType>
struct Polygon {
    std::vector<PointType> vertices;
    std::vector<Polygon> holes;

    template <typename Tx>
    [[nodiscard]] Polygon transform(Tx&& tx) const {
        auto polys = vertices | std::views::transform(tx);
        return {{polys.begin(), polys.end()}};
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

}