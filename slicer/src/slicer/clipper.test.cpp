#include "catch2/generators/catch_generators.hpp"
#include <catch2/catch_test_macros.hpp>

#include "slicer/clipper.hpp"

namespace {

//! Returns a square parallel with the XZ-plane.
[[nodiscard]] slicer::Polygon3D getSquare() {
    return {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}};
}

//! Returns a triangle parallel with the XZ-plane.
[[nodiscard]] slicer::Polygon3D getTrianglePointingDown() {
    return {{{0, 0, 1}, {.5, 0, 0}, {1, 0, 1}}};
}

//! Returns a triangle parallel with the XZ-plane.
[[nodiscard]] slicer::Polygon3D getTrianglePointingUp() {
    return {{{0, 0, 0}, {1, 0, 0}, {.5, 0, 1}}};
}

//! For testing winding-invariance.
[[nodiscard]] slicer::Polygon3D reverseWinding(const slicer::Polygon3D& polygon) {
    auto vertices = polygon.vertices;
    std::ranges::reverse(vertices);
    return {{std::move(vertices)}};
}

//! Compares a and b, including their order, without considering the start position.
bool compareIgnoringStart(const slicer::Polygon3D& a, const slicer::Polygon3D& b) {
    if (a.isEmpty()) {
        return b.isEmpty();
    }

    auto matchesFirst = [&b](const auto& v) { return v == b.vertices.front(); };
    auto v = a.vertices;
    if (auto it = std::ranges::find_if(v, matchesFirst); it != v.end()) {
        std::ranges::rotate(v, it);
    }

    return (slicer::Polygon3D{v} == b);
}

[[nodiscard, maybe_unused]] std::string toString(const slicer::Vec3& pt) {
    return "{" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + ", " + std::to_string(pt.z) + "}";
}

[[nodiscard, maybe_unused]] std::string toString(const slicer::Polygon3D& polygon) {
    auto result = std::string{"["};
    for (const auto& pt : polygon.vertices) {
        result += toString(pt) + ", ";
    }
    result += "]";
    return result;
}

struct ClipUnitTest {
    std::string description;
    slicer::Polygon3D input;
    float zPosition;
    slicer::Polygon3D expectedAbove;
    slicer::Polygon3D expectedBelow;
};

}


TEST_CASE("intersect: vertical line arriving at ZPosition") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{0, 0, 1};
    CHECK(slicer::detail::intersect(p0, p1, 1) == p1);
    CHECK(slicer::detail::intersect(p1, p0, 0) == p0);
}

TEST_CASE("intersect: vertical line starting at ZPosition") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{0, 0, 1};
    CHECK(slicer::detail::intersect(p0, p1, 0) == p0);
    CHECK(slicer::detail::intersect(p1, p0, 1) == p1);
}

TEST_CASE("intersect: vertical line crossing ZPosition") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{0, 0, 1};
    auto intersection = slicer::detail::intersect(p0, p1, 0.5);
    CHECK(intersection == slicer::Vec3{0, 0, .5});
}

TEST_CASE("intersect: diagonal line") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{1, 1, 1};
    auto intersection = slicer::detail::intersect(p0, p1, 0.5);
    CHECK(intersection == slicer::Vec3{.5, .5, .5});
}

TEST_CASE("intersect: negative slope diagonal line") {
    auto p0 = slicer::Vec3{1, 1, 1};
    auto p1 = slicer::Vec3{0, 0, 0};
    auto intersection = slicer::detail::intersect(p0, p1, 0.5);
    CHECK(intersection == slicer::Vec3{.5, .5, .5});
}

TEST_CASE("intersect: diagonal line below 0") {
    auto p0 = slicer::Vec3{-1, -1, -1};
    auto p1 = slicer::Vec3{0, 0, 0};
    auto intersection = slicer::detail::intersect(p0, p1, -0.5);
    CHECK(intersection == slicer::Vec3{-.5, -.5, -.5});
}

TEST_CASE("intersect: negative slope diagonal line below 0") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{-1, -1, -1};
    auto intersection = slicer::detail::intersect(p0, p1, -0.5);
    CHECK(intersection == slicer::Vec3{-.5, -.5, -.5});
}

TEST_CASE("slicer::clip") {
    auto testCase = GENERATE(
        ClipUnitTest{
            .description = "Clip: polygon entirely above zPosition",
            .input = getSquare(),
            .zPosition = -1,
            .expectedAbove = getSquare(),
            .expectedBelow = slicer::Polygon3D{}
        },
        ClipUnitTest{
            .description = "Clip: polygon entirely above zPosition (negative Z)",
            .input = getSquare().translate({0, 0, -1}),
            .zPosition = -2,
            .expectedAbove = getSquare().translate({0, 0, -1}),
            .expectedBelow = slicer::Polygon3D{}
        },
        ClipUnitTest{
            .description = "Clip: polygon entirely below zPosition",
            .input = getSquare(),
            .zPosition = 2,
            .expectedAbove = slicer::Polygon3D{},
            .expectedBelow = getSquare()
        },
        ClipUnitTest{
            .description = "Clip: base of polygon exactly on zPosition",
            .input = getSquare(),
            .zPosition = 0,
            .expectedAbove = getSquare(),
            .expectedBelow = slicer::Polygon3D{}
        },
        ClipUnitTest{
            .description = "Clip: base of polygon exactly on zPosition (negative Z)",
            .input = getSquare().translate({0, 0, -1}),
            .zPosition = -1,
            .expectedAbove = getSquare().translate({0, 0, -1}),
            .expectedBelow = slicer::Polygon3D{}
        },
        ClipUnitTest{
            .description = "Clip: top of polygon exactly on zPosition",
            .input = getSquare(),
            .zPosition = 1,
            .expectedAbove = slicer::Polygon3D{},
            .expectedBelow = getSquare()
        },
        ClipUnitTest{
            .description = "Clip: top of polygon exactly on zPosition (negative Z)",
            .input = getSquare().translate({0, 0, -1}),
            .zPosition = 0,
            .expectedAbove = slicer::Polygon3D{},
            .expectedBelow = getSquare().translate({0, 0, -1})
        },
        ClipUnitTest{
            .description = "Clip: square split by zPosition",
            .input = getSquare(),
            .zPosition = .5,
            .expectedAbove = {{{0, 0, .5}, {1, 0, .5}, {1, 0, 1}, {0, 0, 1}}},
            .expectedBelow = {{{0, 0, 0}, {1, 0, 0}, {1, 0, .5}, {0, 0, .5}}}
        },
        ClipUnitTest{
            .description = "Clip: triangle pointing down",
            .input = getTrianglePointingDown(),
            .zPosition = .5,
            .expectedAbove = {{{.25, 0, .5}, {.75, 0, .5}, {1, 0, 1}, {0, 0, 1}}},
            .expectedBelow = {{{.25, 0, .5}, {.5, 0, 0}, {.75, 0, .5}}}
        },
        ClipUnitTest{
            .description = "Clip: triangle pointing up",
            .input = getTrianglePointingUp(),
            .zPosition = .5,
            .expectedAbove = {{{.25, 0, .5}, {.75, 0, .5}, {.5, 0, 1}}},
            .expectedBelow = {{{0, 0, 0}, {1, 0, 0}, {.75, 0, 0.5}, {.25, 0, .5}}}
        }
    );

    INFO(testCase.description);
    {
        auto input = testCase.input;
        auto aboveRegion = slicer::clip(testCase.input, testCase.zPosition, slicer::KeepRegion::Above);

        INFO("Input: " + toString(input));
        INFO("ZPosition: " + std::to_string(testCase.zPosition));

        INFO("Expected above: " + toString(testCase.expectedAbove));
        INFO("actual above: " + toString(aboveRegion));
        CHECK(compareIgnoringStart(testCase.expectedAbove, aboveRegion));

        auto belowRegion = slicer::clip(input, testCase.zPosition, slicer::KeepRegion::Below);
        INFO("Expected below: " + toString(testCase.expectedBelow));
        INFO("actual below: " + toString(belowRegion));
        CHECK(compareIgnoringStart(testCase.expectedBelow, belowRegion));
    }

    // Testing winding invariance:
    {
        auto reversedInput = reverseWinding(testCase.input);

        INFO("Input (reverse winding): " + toString(reversedInput));
        INFO("ZPosition: " + std::to_string(testCase.zPosition));

        auto aboveRegion = slicer::clip(reversedInput, testCase.zPosition, slicer::KeepRegion::Above);

        auto reverseExpectedAbove = reverseWinding(testCase.expectedAbove);
        INFO("Expected reversed above: " + toString(reverseExpectedAbove));
        INFO("actual reversed above: " + toString(aboveRegion));
        CHECK(compareIgnoringStart(reverseExpectedAbove, aboveRegion));

        auto belowRegion = slicer::clip(reversedInput, testCase.zPosition, slicer::KeepRegion::Below);

        auto reversedExpectedBelow = reverseWinding(testCase.expectedBelow);
        INFO("Expected reversed below: " + toString(reversedExpectedBelow));
        INFO("actual reversed below: " + toString(belowRegion));
        CHECK(compareIgnoringStart(reversedExpectedBelow, belowRegion));
    }
}
