#include <catch2/catch_test_macros.hpp>

#include "slicer/clipper.hpp"

#include <iostream>

namespace {

//! Returns a square parallel with the XZ-plane.
[[nodiscard]] slicer::Polygon3D getSquare() {
    return {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}};
}

//! Returns a triangle parallel with the XZ-plane.
[[nodiscard]] slicer::Polygon3D getTriangle() {
    return {{{0, 0, 1}, {1, 0, 1}, {.5, 0, 0}}};
}

//! For testing winding-invariance.
[[nodiscard]] slicer::Polygon3D reverseWinding(const slicer::Polygon3D& polygon) {
    auto vertices = polygon.vertices;
    std::ranges::reverse(vertices);
    return {{std::move(vertices)}};
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

}

TEST_CASE("intersect: horizontal line on intersect plane") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{1, 0, 0};
    auto intersection = slicer::detail::intersect(p0, p1, 0.);
    CHECK(intersection == slicer::Vec3{0, 0, 0});
}

TEST_CASE("intersect: horizontal line below intersect plane") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{1, 0, 0};
    auto intersection = slicer::detail::intersect(p0, p1, 2.);
    CHECK(intersection == std::nullopt);
}

TEST_CASE("intersect: horizontal line above intersect plane") {
    auto p0 = slicer::Vec3{0, 0, 0};
    auto p1 = slicer::Vec3{1, 0, 0};
    auto intersection = slicer::detail::intersect(p0, p1, -1.);
    CHECK(intersection == std::nullopt);
}

TEST_CASE("intersect: vertical line") {
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

TEST_CASE("Clip: polygon entirely above zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, -1.f, slicer::KeepRegion::Above);
    CHECK(result == inputShape);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, -1.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult == oppositeWindingInputShape);
}

TEST_CASE("Clip: polygon entirely above zPosition, KeepRegion::Above (negative)") {
    auto inputShape = getSquare().translate({0, 0, -1.f});
    auto result = slicer::clip(inputShape, -2.f, slicer::KeepRegion::Above);
    CHECK(result == inputShape);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, -2.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult == oppositeWindingInputShape);
}

TEST_CASE("Clip: base of polygon exactly on zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, 0.f, slicer::KeepRegion::Above);
    CHECK(result == inputShape);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, 0.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult == oppositeWindingInputShape);
}

TEST_CASE("Clip: base of polygon exactly on zPosition, KeepRegion::Above (negative)") {
    auto inputShape = getSquare().translate({0, 0, -1.f});
    auto result = slicer::clip(inputShape, -1.f, slicer::KeepRegion::Above);
    CHECK(result == inputShape);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, -1.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult == oppositeWindingInputShape);
}

TEST_CASE("Clip: polygon entirely below zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, 2.f, slicer::KeepRegion::Above);
    CHECK(result.isEmpty());

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, 2.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult.isEmpty());
}

TEST_CASE("Clip: polygon entirely below zPosition, KeepRegion::Above (negative)") {
    auto inputShape = getSquare().translate({0, 0, -1.f});
    auto result = slicer::clip(inputShape, 1.f, slicer::KeepRegion::Above);
    CHECK(result.isEmpty());

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, 1.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult.isEmpty());
}

TEST_CASE("Clip: top of polygon exactly on zPosition, KeepRegion::Above") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, 1.f, slicer::KeepRegion::Above);
    CHECK(result.isEmpty());

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, 1.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult.isEmpty());
}

TEST_CASE("Clip: top of polygon exactly on zPosition, KeepRegion::Above (negative)") {
    auto inputShape = getSquare().translate({0, 0, -1.f});
    auto result = slicer::clip(inputShape, 0.f, slicer::KeepRegion::Above);
    CHECK(result.isEmpty());

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, 0.f, slicer::KeepRegion::Above);
    CHECK(oppositeWindingResult.isEmpty());
}

TEST_CASE("Clip: square polygon split by zPosition") {
    auto inputShape = getSquare();
    auto result = slicer::clip(inputShape, .5f, slicer::KeepRegion::Above);

    // The winding of the vertices is respected but the order is offset (first >z vertex appears first).
    auto halfSquare = slicer::Polygon3D{{{1, 0, 1}, {0, 0, 1}, {0, 0, .5}, {1, 0, .5}}};
    CHECK(result == halfSquare);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, .5f, slicer::KeepRegion::Above);
    auto halfSquareReverseWinding = slicer::Polygon3D{{{0, 0, 1}, {1, 0, 1}, {1, 0, .5}, {0, 0, .5}}};
    CHECK(oppositeWindingResult == halfSquareReverseWinding);
}

TEST_CASE("Clip: square polygon split by zPosition (negative)") {
    auto inputShape = getSquare().translate({0, 0, -1.f});
    auto result = slicer::clip(inputShape, -.5f, slicer::KeepRegion::Above);
    auto halfSquare = slicer::Polygon3D{{{{1, 0, 0}, {0, 0, 0}, {0, 0, -.5}, {1, 0, -.5}}}};
    CHECK(result == halfSquare);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, -.5f, slicer::KeepRegion::Above);
    auto halfSquareReverseWinding = slicer::Polygon3D{{{{0., 0., 0.}, {1., 0., 0.}, {1., 0., -0.5}, {0., 0., -0.5}}}};
    CHECK(oppositeWindingResult == halfSquareReverseWinding);
}

TEST_CASE("Clip: triangle polygon split by zPosition into a quadrilateral") {
    auto inputShape = getTriangle();
    auto result = slicer::clip(inputShape, .5f, slicer::KeepRegion::Above);
    auto quadrilateral = slicer::Polygon3D{{{{0., 0., 1.}, {1., 0., 1.}, {0.75, 0., 0.5}, {0.25, 0., 0.5}}}};
    CHECK(result == quadrilateral);

    auto oppositeWindingInputShape = reverseWinding(inputShape);
    auto oppositeWindingResult = slicer::clip(oppositeWindingInputShape, .5f, slicer::KeepRegion::Above);
    auto quadrilateralReverseWinding = slicer::Polygon3D{{{{1., 0., 1.}, {0., 0., 1.}, {0.25, 0., 0.5}, {0.75, 0., 0.5}}}};
    CHECK(oppositeWindingResult == quadrilateralReverseWinding);
}
