#include "slicer/slicer.hpp"

#include "slicer/timing.hpp"

namespace slicer {

std::vector<float> getSliceHeights(const BBox3D& volume, float thickness) {
    std::vector<float> result;
    const auto bottom = volume.min.z();

    auto current = bottom;
    while (current < volume.max.z()) {
        result.emplace_back(current);
        current += thickness;
    }
    return result;
}

std::vector<Slice> slice(const I_SpatialIndex& mesh, float thickness) {
    std::vector<Slice> result;

    // For timing each slicing operation at each slice height.
    timing::LabelToAccumulatedDuration<std::chrono::microseconds> accumulatedDurations;

    auto time = timing::Clock::now();

    const auto sliceHeights = getSliceHeights(mesh.AABB(), thickness);
    std::cout << "Num slices: " << sliceHeights.size() << std::endl << std::endl;

    for (const auto& sliceHeight : sliceHeights) {
        auto triangles = mesh.query(sliceHeight);
        timing::timeAndStore(time, "1: query spatial index", accumulatedDurations);

        auto intersectData = intersect(triangles, sliceHeight);
        timing::timeAndStore(time, "2: intersect triangles", accumulatedDurations);

        auto adjacencyList = getManifoldAdjacencyList(intersectData);
        timing::timeAndStore(time, "3: build adjacency list", accumulatedDurations);

        auto outlines = getSliceOutlines(adjacencyList);
        timing::timeAndStore(time, "4: get slice outlines", accumulatedDurations);

        auto relativeOutlines = identifyWindings(outlines);
        timing::timeAndStore(time, "5: indentify windings", accumulatedDurations);

        auto outlineHierarchy = OutlineHierarchy{std::move(relativeOutlines)};
        timing::timeAndStore(time, "6: get outline hierarchy", accumulatedDurations);

        auto polygons =  outlineHierarchy.getPolygons();
        timing::timeAndStore(time, "7: get polygons", accumulatedDurations);

        result.push_back({{polygons.begin(), polygons.end()}, sliceHeight});
    }

    timing::logTimings("Slice Operations", accumulatedDurations, result.size());

    return result;
}

}
