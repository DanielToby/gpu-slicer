#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>

#include "geometry/clipper.hpp"
#include "geometry/intersect.hpp"

using namespace slicer;

namespace {

//! For testing winding invariance.
[[nodiscard]] Polygon3D reverseWinding(const Polygon3D& polygon) {
    auto vertices = polygon.vertices;
    std::ranges::reverse(vertices);
    return {{std::move(vertices)}};
}

//! For testing first-vertex invariance.
[[nodiscard]] std::vector<Polygon3D> getAllStartVertexCombinations(const Polygon3D& polygon) {
    std::vector<Polygon3D> result;
    for (auto i = 0; i < polygon.vertices.size(); ++i) {
        auto copy = polygon;
        std::ranges::rotate(copy.vertices, copy.vertices.begin() + i);
        result.emplace_back(std::move(copy));
    }
    return result;
}

//! Compares a and b, including their order, without considering the start position.
bool compareIgnoringStart(const Polygon3D& a, const Polygon3D& b) {
    if (a.vertices.size() != b.vertices.size()) {
        return false;
    }
    if (a.isEmpty()) {
        return true;
    }

    auto matchesFirst = [&b](const auto& v) { return v == b.vertices.front(); };
    auto v = a.vertices;
    if (auto it = std::ranges::find_if(v, matchesFirst); it != v.end()) {
        std::ranges::rotate(v, it);
    }

    return (Polygon3D{v} == b);
}

[[nodiscard, maybe_unused]] std::string toString(const Vec3& pt) {
    return "{" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + ", " + std::to_string(pt.z) + "}";
}

[[nodiscard, maybe_unused]] std::string toString(const Polygon3D& polygon) {
    auto result = std::string{"["};
    for (const auto& pt : polygon.vertices) {
        result += toString(pt) + ", ";
    }
    result += "]";
    return result;
}

struct LineBehaviorUnitTest {
    std::string description;
    Vec3 p0;
    Vec3 p1;
    float zPosition;
    KeepRegion keepRegion;
    detail::LineBehavior expectedBehavior;
};

struct ClipUnitTest {
    std::string description;
    Polygon3D input;
    float zPosition;
    Polygon3D expectedAbove;
    Polygon3D expectedBelow;
};

}

TEST_CASE("lineBehavior") {
    using detail::LineBehavior;

    auto testCase = GENERATE(
        LineBehaviorUnitTest{
            .description = "p0 is in, p1 is in (above)",
            .p0 = {0, 0, 1},
            .p1 = {0, 0, 2},
            .zPosition = 0,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::RemainsIn
        },
        LineBehaviorUnitTest{
            .description = "p0 is in, p1 is in (below)",
            .p0 = {0, 0, 1},
            .p1 = {0, 0, 2},
            .zPosition = 3,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::RemainsIn
        },
        LineBehaviorUnitTest{
            .description = "p0 is on, p1 is on (above)",
            .p0 = {0, 0, 1},
            .p1 = {1, 0, 1},
            .zPosition = 1,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::RemainsIn
        },
        LineBehaviorUnitTest{
            .description = "p0 is on, p1 is on (below)",
            .p0 = {0, 0, 1},
            .p1 = {1, 0, 1},
            .zPosition = 1,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::RemainsIn
        },
        LineBehaviorUnitTest{
            .description = "p0 is in, p1 is out (above)",
            .p0 = {0, 0, 2},
            .p1 = {0, 0, 0},
            .zPosition = 1,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::Exits
        },
        LineBehaviorUnitTest{
            .description = "p0 is in, p1 is out (below)",
            .p0 = {0, 0, 0},
            .p1 = {0, 0, 2},
            .zPosition = 1,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::Exits
        },
        LineBehaviorUnitTest{
            .description = "p0 is on, p1 is out (above)",
            .p0 = {0, 0, 1},
            .p1 = {0, 0, 0},
            .zPosition = 1,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::Exits
        },
        LineBehaviorUnitTest{
            .description = "p0 is on, p1 is out (below)",
            .p0 = {0, 0, 0},
            .p1 = {0, 0, 1},
            .zPosition = 0,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::Exits
        },
        LineBehaviorUnitTest{
            .description = "p0 is out, p1 is out (above)",
            .p0 = {0, 0, 0},
            .p1 = {0, 0, 1},
            .zPosition = 2,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::RemainsOut
        },
        LineBehaviorUnitTest{
            .description = "p0 is out, p1 is out (below)",
            .p0 = {0, 0, 2},
            .p1 = {0, 0, 1},
            .zPosition = 0,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::RemainsOut
        },
        LineBehaviorUnitTest{
            .description = "p0 is out, p1 is in (above)",
            .p0 = {0, 0, 0},
            .p1 = {0, 0, 2},
            .zPosition = 1,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::Enters
        },
        LineBehaviorUnitTest{
            .description = "p0 is out, p1 is in (below)",
            .p0 = {0, 0, 2},
            .p1 = {0, 0, 0},
            .zPosition = 1,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::Enters
        },
        LineBehaviorUnitTest{
            .description = "p0 is out, p1 is on (above)",
            .p0 = {0, 0, 0},
            .p1 = {0, 0, 2},
            .zPosition = 2,
            .keepRegion = KeepRegion::Above,
            .expectedBehavior = LineBehavior::RemainsOut
        },
        LineBehaviorUnitTest{
            .description = "p0 is out, p1 is on (below)",
            .p0 = {0, 0, 2},
            .p1 = {0, 0, 0},
            .zPosition = 0,
            .keepRegion = KeepRegion::Below,
            .expectedBehavior = LineBehavior::RemainsOut
        }
    );

    INFO(testCase.description);
    CHECK(detail::lineBehavior(
              testCase.p0,
              testCase.p1,
              testCase.zPosition,
              testCase.keepRegion) == testCase.expectedBehavior);
}

 TEST_CASE("clip") {
     auto testCase = GENERATE(
         ClipUnitTest{
             .description = "Clip: polygon entirely above zPosition",
             .input = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .zPosition = -1,
             .expectedAbove = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .expectedBelow = Polygon3D{}
         },
         ClipUnitTest{
             .description = "Clip: polygon entirely above zPosition (negative Z)",
             .input = {{{0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {0, 0, 0}}},
             .zPosition = -2,
             .expectedAbove = {{{0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {0, 0, 0}}},
             .expectedBelow = Polygon3D{}
         },
         ClipUnitTest{
             .description = "Clip: polygon entirely below zPosition",
             .input = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .zPosition = 2,
             .expectedAbove = Polygon3D{},
             .expectedBelow = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}}
         },
         ClipUnitTest{
             .description = "Clip: base of polygon exactly on zPosition",
             .input = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .zPosition = 0,
             .expectedAbove = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .expectedBelow = Polygon3D{}
         },
         ClipUnitTest{
             .description = "Clip: base of polygon exactly on zPosition (negative Z)",
             .input = {{{0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {0, 0, 0}}},
             .zPosition = -1,
             .expectedAbove = {{{0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {0, 0, 0}}},
             .expectedBelow = Polygon3D{}
         },
         ClipUnitTest{
             .description = "Clip: top of polygon exactly on zPosition",
             .input = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .zPosition = 1,
             .expectedAbove = Polygon3D{},
             .expectedBelow = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}}
         },
         ClipUnitTest{
             .description = "Clip: top of polygon exactly on zPosition (negative Z)",
             .input = {{{0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {0, 0, 0}}},
             .zPosition = 0,
             .expectedAbove = Polygon3D{},
             .expectedBelow = {{{0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {0, 0, 0}}}
         },
         ClipUnitTest{
             .description = "Clip: square split by zPosition",
             .input = {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
             .zPosition = .5,
             .expectedAbove = {{{0, 0, .5}, {1, 0, .5}, {1, 0, 1}, {0, 0, 1}}},
             .expectedBelow = {{{0, 0, 0}, {1, 0, 0}, {1, 0, .5}, {0, 0, .5}}}
         },
         ClipUnitTest{
             .description = "Clip: triangle pointing down",
             .input = {{{0, 0, 1}, {.5, 0, 0}, {1, 0, 1}}},
             .zPosition = .5,
             .expectedAbove = {{{.25, 0, .5}, {.75, 0, .5}, {1, 0, 1}, {0, 0, 1}}},
             .expectedBelow = {{{.25, 0, .5}, {.5, 0, 0}, {.75, 0, .5}}}
         },
         ClipUnitTest{
             .description = "Clip: triangle pointing up",
             .input = {{{0, 0, 0}, {1, 0, 0}, {.5, 0, 1}}},
             .zPosition = .5,
             .expectedAbove = {{{.25, 0, .5}, {.75, 0, .5}, {.5, 0, 1}}},
             .expectedBelow = {{{0, 0, 0}, {1, 0, 0}, {.75, 0, 0.5}, {.25, 0, .5}}}
         },
         ClipUnitTest{
             .description = "Clip: negative right triangle pointing up",
             .input = {{{-2, 0, -2}, {-1, 0, -2}, {-1, 0, -1}}},
             .zPosition = -1.5,
             .expectedAbove = {{{-1.5, 0, -1.5}, {-1, 0, -1.5}, {-1, 0, -1}}},
             .expectedBelow = {{{-2, 0, -2}, {-1, 0, -2}, {-1, 0, -1.5}, {-1.5, 0, -1.5}}}
         },
         ClipUnitTest{
             .description = "Clip: triangle pointing down with change in y-axis",
             .input = {{{0, 0, 0}, {2, 2, 2}, {0, 0, 2}}},
             .zPosition = 1,
             .expectedAbove = {{{0, 0, 1}, {1, 1, 1}, {2, 2, 2}, {0, 0, 2}}},
             .expectedBelow = {{{0, 0, 0}, {1, 1, 1}, {0, 0, 1}}}
         },
         ClipUnitTest{
             .description = "Clip: square with two vertices on the slice plane",
             .input = {{{0, 0, 0}, {1, 0, -1}, {2, 0, 0}, {1, 0, 1}}},
             .zPosition = 0,
             .expectedAbove = {{{0, 0, 0}, {2, 0, 0}, {1, 0, 1}}},
             .expectedBelow = {{{0, 0, 0}, {1, 0, -1}, {2, 0, 0}}}
         },
         ClipUnitTest{
             .description = "Clip: hexagon with two vertices on the slice plane",
             .input = {{{0, 0, 0}, {1, 0, -1}, {2, 0, -1}, {3, 0, 0}, {2, 0, 1}, {1, 0, 1}}},
             .zPosition = 0,
             .expectedAbove = {{{0, 0, 0}, {3, 0, 0}, {2, 0, 1}, {1, 0, 1}}},
             .expectedBelow = {{{0, 0, 0}, {1, 0, -1}, {2, 0, -1}, {3, 0, 0}}}
         },
         ClipUnitTest{
             .description = "Clip: triangle above slice plane with two vertices on the slice plane",
             .input = {{{0, 0, 0}, {2, 0, 0}, {2, 0, 2}}},
             .zPosition = 0,
             .expectedAbove = {{{0, 0, 0}, {2, 0, 0}, {2, 0, 2}}},
             .expectedBelow = Polygon3D{}
         },
         ClipUnitTest{
             .description = "Clip: triangle above slice plane with one vertex on the slice plane",
             .input = {{{0, 0, 2}, {0, 0, 0}, {2, 0, 2}}},
             .zPosition = 0,
             .expectedAbove = {{{0, 0, 2}, {0, 0, 0}, {2, 0, 2}}},
             .expectedBelow = Polygon3D{}
         },
         ClipUnitTest{
             .description = "Clip: triangle below slice plane with two vertices on the slice plane",
             .input = {{{0, 0, 2}, {0, 0, 0}, {2, 0, 2}}},
             .zPosition = 2,
             .expectedAbove = Polygon3D{},
             .expectedBelow = {{{0, 0, 2}, {0, 0, 0}, {2, 0, 2}}}
         },
         ClipUnitTest{
             .description = "Clip: triangle below slice plane with one vertex on the slice plane",
             .input = {{{0, 0, 0}, {2, 0, 0}, {2, 0, 2}}},
             .zPosition = 2,
             .expectedAbove = Polygon3D{},
             .expectedBelow = {{{0, 0, 0}, {2, 0, 0}, {2, 0, 2}}}
         }
     );

     INFO(testCase.description);
     {
         for (const auto& input : getAllStartVertexCombinations(testCase.input)) {
             INFO("Input: " + toString(input));
             INFO("ZPosition: " + std::to_string(testCase.zPosition));

             auto aboveRegion = clip(testCase.input, testCase.zPosition, KeepRegion::Above);

             INFO("Expected above: " + toString(testCase.expectedAbove));
             INFO("actual above: " + toString(aboveRegion));
             CHECK(compareIgnoringStart(testCase.expectedAbove, aboveRegion));

             auto belowRegion = clip(input, testCase.zPosition, KeepRegion::Below);

             INFO("Expected below: " + toString(testCase.expectedBelow));
             INFO("actual below: " + toString(belowRegion));
             CHECK(compareIgnoringStart(testCase.expectedBelow, belowRegion));
         }
     }

     // Testing winding invariance:
     {
         auto reversedInput = reverseWinding(testCase.input);
         for (const auto& input : getAllStartVertexCombinations(reversedInput)) {
             INFO("Input (reverse winding): " + toString(reversedInput));
             INFO("ZPosition: " + std::to_string(testCase.zPosition));

             auto aboveRegion = clip(reversedInput, testCase.zPosition, KeepRegion::Above);

             auto reverseExpectedAbove = reverseWinding(testCase.expectedAbove);
             INFO("Expected reversed above: " + toString(reverseExpectedAbove));
             INFO("actual reversed above: " + toString(aboveRegion));
             CHECK(compareIgnoringStart(reverseExpectedAbove, aboveRegion));

             auto belowRegion = clip(reversedInput, testCase.zPosition, KeepRegion::Below);

             auto reversedExpectedBelow = reverseWinding(testCase.expectedBelow);
             INFO("Expected reversed below: " + toString(reversedExpectedBelow));
             INFO("actual reversed below: " + toString(belowRegion));
             CHECK(compareIgnoringStart(reversedExpectedBelow, belowRegion));
         }
     }
}
