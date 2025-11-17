#include <catch2/catch_test_macros.hpp>

#include "slicer/intersect.hpp"

using namespace slicer;

TEST_CASE("intersect: vertical line arriving at ZPosition") {
    auto p0 = QuantizedPoint3D::fromPoint({0, 0, 0});
    auto p1 = QuantizedPoint3D::fromPoint({0, 0, 1});
    CHECK(intersect(QuantizedLine3D{p0, p1}, 1) == p1);
    CHECK(intersect(QuantizedLine3D{p1, p0}, 0) == p0);
}

TEST_CASE("intersect: vertical line starting at ZPosition") {
    auto p0 = QuantizedPoint3D::fromPoint({0, 0, 0});
    auto p1 = QuantizedPoint3D::fromPoint({0, 0, 1});
    CHECK(intersect(QuantizedLine3D{p0, p1}, 0) == p0);
    CHECK(intersect(QuantizedLine3D{p1, p0}, 1) == p1);
}

TEST_CASE("intersect: vertical line crossing ZPosition") {
    auto p0 = QuantizedPoint3D::fromPoint({0, 0, 0});
    auto p1 = QuantizedPoint3D::fromPoint({0, 0, 1});
    auto intersection = intersect(QuantizedLine3D{p0, p1}, 0.5);
    CHECK(intersection->value() == Vec3{0, 0, .5});
}

TEST_CASE("intersect: diagonal line") {
    auto p0 = QuantizedPoint3D::fromPoint({0, 0, 0});
    auto p1 = QuantizedPoint3D::fromPoint({1, 1, 1});
    auto intersection = intersect(QuantizedLine3D{p0, p1}, 0.5);
    CHECK(intersection->value() == Vec3{.5, .5, .5});
}

TEST_CASE("intersect: negative slope diagonal line") {
    auto p0 = QuantizedPoint3D::fromPoint({1, 1, 1});
    auto p1 = QuantizedPoint3D::fromPoint({0, 0, 0});
    auto intersection = intersect(QuantizedLine3D{p0, p1}, 0.5);
    CHECK(intersection->value() == Vec3{.5, .5, .5});
}

TEST_CASE("intersect: diagonal line below 0") {
    auto p0 = QuantizedPoint3D::fromPoint({-1, -1, -1});
    auto p1 = QuantizedPoint3D::fromPoint({0, 0, 0});
    auto intersection = intersect(QuantizedLine3D{p0, p1}, -0.5);
    CHECK(intersection->value() == Vec3{-.5, -.5, -.5});
}

TEST_CASE("intersect: negative slope diagonal line below 0") {
    auto p0 =QuantizedPoint3D::fromPoint({0, 0, 0});
    auto p1 =QuantizedPoint3D::fromPoint({-1, -1, -1});
    auto intersection = intersect(QuantizedLine3D{p0, p1}, -0.5);
    CHECK(intersection->value() == Vec3{-.5, -.5, -.5});
}
