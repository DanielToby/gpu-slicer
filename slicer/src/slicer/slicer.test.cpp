#include <catch2/catch_test_macros.hpp>

#include "slicer/slicer.hpp"
#include "slicer/stl.hpp"

#include <iostream>

TEST_CASE("getSliceHeights: thickness smaller than bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 1.) == std::vector<slicer::SliceDimensions>{
                                                   {-2., -1.},
                                                   {-1., 0.},
                                                   {0., 1.},
                                                   {1., 2.}});
}

TEST_CASE("getSliceHeights: thickness equal to bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 4.) == std::vector<slicer::SliceDimensions>{{-2., 2.}});
}

TEST_CASE("getSliceHeights: thickness larger than bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 5.) == std::vector<slicer::SliceDimensions>{{-2., 2.}});
}

TEST_CASE("slice") {
    auto mesh = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");

    // min: -29.176 -15.502 0
    // max: 30.825 15.502 48
    // z span: [0, 48]

    auto slices = slicer::slice(mesh, 2.);
    CHECK(slices.size() == 24);
}
