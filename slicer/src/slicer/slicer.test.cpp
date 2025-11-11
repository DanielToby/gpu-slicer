#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <iostream>

#include "slicer/slicer.hpp"
#include "slicer/stl.hpp"
#include "slicer/svg.hpp"
#include "spatial_index/no_spatial_index.hpp"

namespace {

template <typename Fn>
void logTimedExecution(std::string&& message, Fn&& fn) {
    using namespace std::chrono;
    const auto start = high_resolution_clock::now();

    std::invoke(fn);

    const auto stop = high_resolution_clock::now();
    const auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << message << " took: " << duration.count() << " ms" << std::endl;
}

}

TEST_CASE("getSliceHeights: thickness smaller than bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 1.) == std::vector<slicer::Span>{
                                                   {-2., -1.},
                                                   {-1., 0.},
                                                   {0., 1.},
                                                   {1., 2.}});
}

TEST_CASE("getSliceHeights: thickness equal to bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 4.) == std::vector<slicer::Span>{{-2., 2.}});
}

TEST_CASE("getSliceHeights: thickness larger than bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 5.) == std::vector<slicer::Span>{{-2., 2.}});
}

TEST_CASE("slice") {
    auto triangles = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");

    // min: -29.176 -15.502 0
    // max: 30.825 15.502 48
    // z span: [0, 48]

    auto noSpatialIndex = slicer::NoSpatialIndex{};
    logTimedExecution("Build spatial index (none)", [&] {
        noSpatialIndex.build(triangles);
    });

    std::vector<slicer::Slice> slices;
    logTimedExecution("Slice (no spatial index)", [&] {
        slices = slicer::slice(noSpatialIndex, .5);
    });

    auto bbox = noSpatialIndex.AABB();
    auto dimensions = slicer::BBox2D{slicer::toVec2(bbox.min), slicer::toVec2(bbox.max)};
    for (auto i = 0; i < slices.size(); i++) {
        auto path = "/Users/daniel.toby/Desktop/output/" + std::to_string(i) + ".svg";
        slicer::writeSVG(dimensions, slices[i].polygons, path);
    }
}
