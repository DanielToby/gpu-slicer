#include <catch2/catch_test_macros.hpp>

#include "slicer/mesh.hpp"
#include "slicer/stl.hpp"

TEST_CASE("getAABB") {
    auto mesh = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");
    auto bbox = slicer::getAABB(mesh);
    CHECK(!bbox.empty());
}