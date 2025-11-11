#include <catch2/catch_test_macros.hpp>

#include "slicer/stl.hpp"

TEST_CASE("Load .stl file") {
    const auto triangles = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");
    CHECK_FALSE(triangles.empty());
}
