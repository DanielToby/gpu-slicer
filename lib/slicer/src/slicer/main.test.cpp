#include <catch2/catch_test_macros.hpp>

#include "slicer/slicer.hpp"
#include "slicer/stl.hpp"
#include "slicer/svg.hpp"
#include "slicer/timing.hpp"
#include "spatial_index/no_spatial_index.hpp"
#include "spatial_index/bvh.hpp"

using namespace slicer;

namespace {

[[maybe_unused]] void writeSlicesToSVGs(const BBox2D& dimensions, std::span<const Slice> slices) {
    for (auto i = 0; i < slices.size(); i++) {
        auto path = "/Users/daniel.toby/Desktop/output/" + std::to_string(i) + ".svg";
        writeSVG(dimensions, slices[i].polygons, path, /* scaleFactor = */ 40);
    }
}

[[nodiscard]] std::string getStlFilePath() {
#ifdef _WIN32
    return R"(C:\Users\Daniel Toby\Documents\3DBenchy.stl)";
#elif __APPLE__
    return R"(/Users/daniel.toby/Desktop/3DBenchy.stl)";
#endif
}

}

TEST_CASE("Slice 3DBenchy") {
    auto allOperationTimes = timing::LabelToAccumulatedDuration<std::chrono::milliseconds>{};

    const auto triangles = loadStl(getStlFilePath());

    // No spatial index:
    {
        std::cout << "Run 1: No Spatial Index, " << triangles.size() << " triangles:" << std::endl;

        auto time = timing::Clock::now();
        auto localTimes = timing::LabelToAccumulatedDuration<std::chrono::milliseconds>{};

        auto noSpatialIndex = NoSpatialIndex{};
        noSpatialIndex.build(triangles);
        timing::timeAndStore(time, "build spatial index (none)", localTimes);

        auto slices = slice(noSpatialIndex, .5);
        timing::timeAndStore(time, "slice (no spatial index)", localTimes);

        timing::logTimings("Total Run (no spatial index)", localTimes);
    }

    // Bounding Volume Hierarchy (BVH):
    {
        std::cout << "Run 2: Bounding Volume Hierarchy, " << triangles.size() << " triangles:" << std::endl;

        auto time = timing::Clock::now();
        auto localTimes = timing::LabelToAccumulatedDuration<std::chrono::milliseconds>{};

        auto bvh = BVH{ConstructionStrategy::BinarySplit};
        bvh.build(triangles);
        timing::timeAndStore(time, "build spatial index (BVH)", localTimes);

        auto slices = slice(bvh, .5);
        timing::timeAndStore(time, "slice (BVH)", localTimes);

        timing::logTimings("Total Run (BVH)", localTimes);
    }
}
