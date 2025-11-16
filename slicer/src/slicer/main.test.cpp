#include <catch2/catch_test_macros.hpp>

#include "slicer/slicer.hpp"
#include "slicer/stl.hpp"
#include "slicer/svg.hpp"
#include "slicer/timing.hpp"
#include "spatial_index/no_spatial_index.hpp"

TEST_CASE("Slice 3DBenchy") {
    slicer::timing::LabelToAccumulatedDuration<std::chrono::milliseconds> accumulatedDurations;
    auto time = slicer::timing::Clock::now();

    const auto triangles = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");
    slicer::timing::timeAndStore(time, "load stl", accumulatedDurations);

    std::cout << "Num triangles: " << triangles.size() << std::endl;

    auto noSpatialIndex = slicer::NoSpatialIndex{};
    noSpatialIndex.build(triangles);
    slicer::timing::timeAndStore(time, "build spatial index (none)", accumulatedDurations);

    auto slices = slicer::slice(noSpatialIndex, .5);
    slicer::timing::timeAndStore(time, "slice (no spatial index)", accumulatedDurations);

    slicer::timing::logTimings("Total Run", accumulatedDurations);

    const auto dimensions = slicer::toBBox2D(noSpatialIndex.AABB());
    for (auto i = 0; i < slices.size(); i++) {
        auto path = "/Users/daniel.toby/Desktop/output/" + std::to_string(i) + ".svg";
        slicer::writeSVG(dimensions, slices[i].polygons, path, /* scaleFactor = */ 40);
    }
}
