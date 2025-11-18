#include "catch2/generators/catch_generators.hpp"

#include <catch2/catch_test_macros.hpp>

#include "slicer/intersect.hpp"

using namespace slicer;

struct IntersectLineTestCase {
    Vec3 p0;
    Vec3 p1;
    float zPosition;
    std::optional<Vec3> expected;
};

TEST_CASE("Line and Segment intersection") {
    auto testCase = GENERATE(
        IntersectLineTestCase{{-1, -1, -1}, {1, 1, 1}, -1, std::nullopt},
        IntersectLineTestCase{{-1, -1, -1}, {1, 1, 1}, 0, Vec3{0, 0, 0}},
        IntersectLineTestCase{{-1, -1, -1}, {1, 1, 1}, 1, std::nullopt},
        IntersectLineTestCase{{0, 0, 0}, {2, 2, 2}, 1, Vec3{1, 1, 1}},
        IntersectLineTestCase{{0, 0, 0}, {2, 2, 2}, -1, std::nullopt}
    );

    auto line = Segment3D{testCase.p0, testCase.p1};
    auto result = intersect(line, testCase.zPosition);
    CHECK(result == testCase.expected);
}
