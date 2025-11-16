#include "slicer/slicer.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <map>

#include "clipper.hpp"

namespace slicer {

namespace {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using LabelToAccumulatedDuration = std::map<std::string, std::chrono::microseconds>;

void timeAndStore(TimePoint& lastTimePoint, const std::string& label, LabelToAccumulatedDuration& durations) {
    const auto now = Clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - lastTimePoint);
    lastTimePoint = now;
    durations[label] += elapsed;
}

void logAverageTimings(const LabelToAccumulatedDuration& labelToAccumulatedDuration, std::size_t numSamples) {
    for (const auto& [label, accumulatedDuration] : labelToAccumulatedDuration) {
        std::cout << "[" << label << "] time: " << accumulatedDuration.count() / numSamples << " μs" << std::endl;
    }
}

}

std::vector<float> getSliceHeights(const BBox3D& volume, float thickness) {
    std::vector<float> result;
    const auto bottom = volume.min.z;

    auto current = bottom;
    while (current < volume.max.z) {
        result.emplace_back(current);
        current += thickness;
    }
    return result;
}

std::vector<Slice> slice(const I_SpatialIndex& mesh, float thickness) {
    std::vector<Slice> result;

    // For timing each slicing operation at each slice height.
    LabelToAccumulatedDuration accumulatedDurations;

    auto time = Clock::now();

    for (const auto& sliceHeight : getSliceHeights(mesh.AABB(), thickness)) {
        auto triangles = mesh.query(sliceHeight);
        timeAndStore(time, "1: query spatial index", accumulatedDurations);

        auto segments = intersect(triangles, sliceHeight);
        timeAndStore(time, "2: intersect triangles", accumulatedDurations);

        auto adjacencyList = getManifoldAdjacencyList(segments);
        timeAndStore(time, "3: build adjacency list", accumulatedDurations);

        auto outlines = getSliceOutlines(adjacencyList);
        timeAndStore(time, "4: get slice outlines", accumulatedDurations);

        auto relativeOutlines = identifyWindings(outlines);
        timeAndStore(time, "5: indentify windings", accumulatedDurations);

        auto outlineHierarchy = OutlineHierarchy{std::move(relativeOutlines)};
        timeAndStore(time, "6: get outline hierarchy", accumulatedDurations);

        auto polygons =  outlineHierarchy.getPolygons();
        timeAndStore(time, "7: get polygons", accumulatedDurations);

        result.push_back({{polygons.begin(), polygons.end()}, sliceHeight});
    }

    logAverageTimings(accumulatedDurations, result.size());

    return result;
}

}
