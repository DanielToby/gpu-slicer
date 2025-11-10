#include <catch2/catch_test_macros.hpp>

#include "slicer/bbox.hpp"

TEST_CASE("BBox: default constructed BBox is empty") {
    auto empty2D = slicer::BBox2D{};
    CHECK(empty2D.empty());

    auto empty3D = slicer::BBox3D{};
    CHECK(empty3D.empty());
}
