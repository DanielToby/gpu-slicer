#include <catch2/catch_test_macros.hpp>

#include "slicer/stl.hpp"

TEST_CASE("Load .stl file") {
    auto triangles = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");
    REQUIRE(triangles.size() > 0);
}
