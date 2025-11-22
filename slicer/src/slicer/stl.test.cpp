#include <catch2/catch_test_macros.hpp>

#include "slicer/stl.hpp"

namespace {

[[nodiscard]] std::string getStlFilePath() {
#ifdef _WIN32
    return R"(C:\Users\Daniel Toby\Documents\3DBenchy.stl)";
#elif __APPLE__
    return R"(/Users/daniel.toby/Desktop/3DBenchy.stl)";
#endif
}

}

TEST_CASE("Load .stl file") {
    const auto triangles = slicer::loadStl(getStlFilePath());
    CHECK_FALSE(triangles.empty());
}
