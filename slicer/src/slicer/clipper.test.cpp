#include <catch2/catch_test_macros.hpp>

#include "slicer/clipper.hpp"

namespace {

//! Returns a square parallel with the Y-plane.
[[nodiscard]] constexpr slicer::Polygon3D getSquare() {
    return {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}};
}

}

TEST_CASE("Clip: polygon entirely above zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, -1.f, slicer::KeepRegion::Above);
    CHECK(result == inputShape);
}

TEST_CASE("Clip: base of polygon exactly on zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, 0.f, slicer::KeepRegion::Above);
    CHECK(result == inputShape);
}

TEST_CASE("Clip: polygon entirely below zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, 2.f, slicer::KeepRegion::Above);
    CHECK(result.isEmpty());
}

TEST_CASE("Clip: top of polygon exactly on zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, 1.f, slicer::KeepRegion::Above);
    CHECK(result.isEmpty());
}

TEST_CASE("Clip: polygon split by zPosition") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, .5f, slicer::KeepRegion::Above);
    auto halfSquare = slicer::Polygon3D{{{0, 0, .5}, {1, 0, .5}, {1, 0, 1}, {0, 0, 1}}};
}