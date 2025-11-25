#include "catch2/generators/catch_generators.hpp"

#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include "geometry/intersect.hpp"

using namespace slicer;

struct IntersectLineTestCase {
    Vec3 p0;
    Vec3 p1;
    float zPosition;
    std::optional<Vec3> expected;
};

std::ostream& operator<<(std::ostream& os, const std::optional<Vec3>& v) {
    if (v) {
        return os << *v;
    }
    return os << "<null>";
}

TEST_CASE("Geometry types are POD") {
    static_assert(std::is_pod<Vec3>::value, "Vec3 is not pod");
}

TEST_CASE("Line and zPosition intersection") {
    auto testCase = GENERATE(
        IntersectLineTestCase{{-1, -1, -1}, {1, 1, 1}, -1, Vec3{-1, -1, -1}},
        IntersectLineTestCase{{-1, -1, -1}, {1, 1, 1}, 0, Vec3{0, 0, 0}},
        IntersectLineTestCase{{-1, -1, -1}, {1, 1, 1}, 1, Vec3{1, 1, 1}},
        IntersectLineTestCase{{0, 0, 0}, {2, 2, 2}, 1, Vec3{1, 1, 1}},
        IntersectLineTestCase{{0, 0, 0}, {2, 2, 2}, -1, std::nullopt});

    auto line = Segment3D{testCase.p0, testCase.p1};
    INFO(line);
    INFO(testCase.zPosition);

    auto result = intersect(line, testCase.zPosition);
    CHECK(result == testCase.expected);
}

struct IntersectTriangleTestCase {
    Triangle3D triangle;
    float zPosition;
    std::optional<Segment3D> expected;
};

TEST_CASE("Triangle and zPosition intersection") {
    auto testCase = GENERATE(
        // Triangle completely above plane → no intersection
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, 5}, {1, 0, 5}, {0, 1, 5}},
            0.0f,
            std::nullopt},

        // Triangle completely below plane → no intersection
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, -5}, {1, 0, -5}, {0, 1, -5}},
            0.0f,
            std::nullopt},

        // Clean slice through two edges → single segment (Z = 0)
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, -1}, {1, 0, 1}, {0, 1, 1}},
            0.0f,
            Segment3D{
                {0.5f, 0.0f, 0.0f},
                {0.0f, 0.5f, 0.0f}}},

        // Plane touches a vertex: choose policy (commonly → no segment)
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, 0}, {1, 0, 1}, {0, 1, 1}},
            0.0f,
            std::nullopt},

        // One triangle edge lies exactly on the slicing plane Z=0
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, 0}, {1, 0, 0}, {0, 1, 1}},
            0.0f,
            Segment3D{
                {0.0f, 0.0f, 0.0f},
                {1.0f, 0.0f, 0.0f}}},

        // Complete coplanar triangle (usually ignored)
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}},
            0.0f,
            std::nullopt},

        // More complex slice through obtuse triangle
        // Intersection points computed parametrically.
        IntersectTriangleTestCase{
            Triangle3D{{0, 0, -2}, {4, 0, 2}, {0, 4, 2}},
            0.0f,
            Segment3D{{2, 0, 0}, {0, 2, 0}}});

    INFO(testCase.triangle);
    INFO(testCase.zPosition);

    auto result = intersect(testCase.triangle, testCase.zPosition);

    CHECK(result == testCase.expected);
}
