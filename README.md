# gpu-slicer

A demo 3D slicer that works on manifold meshes, and a few
extras ([clipper.hpp](https://github.com/DanielToby/gpu-slicer/blob/main/slicer/src/slicer/clipper.hpp)...) that I wrote
mistakenly along the way but didn't have the heart to remove.

![Input](assets/input.png)
![Output](assets/output.gif)

### This Project

The goal of this project is to use a 3D slicing pipeline to benchmark different spatial index implementations, including GPU-accelerated ones.

### How to Run

This project only requires C++20 and CMake >=3.29 to run. The tests are under target `slicer_tests`.

### Results

```
Run 1: No Spatial Index, 225154 triangles:
Num slices: 96

Slice Operations:
[1: query spatial index] time: 3959μs
[2: intersect triangles] time: 774μs
[3: build adjacency list] time: 1083μs
[4: get slice outlines] time: 586μs
[5: identify windings] time: 49μs
[6: get outline hierarchy] time: 146μs
[7: get polygons] time: 16μs

Total Run (no spatial index):
[build spatial index (none)] time: 1ms
[slice (no spatial index)] time: 635ms


Run 2: Bounding Volume Hierarchy, 225154 triangles:
Num slices: 96

Slice Operations:
[1: query spatial index] time: 349μs
[2: intersect triangles] time: 765μs
[3: build adjacency list] time: 1047μs
[4: get slice outlines] time: 588μs
[5: identify windings] time: 48μs
[6: get outline hierarchy] time: 141μs
[7: get polygons] time: 15μs

Total Run (BVH):
[build spatial index (BVH)] time: 439ms
[slice (BVH)] time: 284ms
```

Without a spatial index, querying for triangles intersecting Z takes up most of the slicing time.
With a basic BVH that splits triangles into two even groups, triangle query time is an order of magnitude faster.

### The Spatial Index

This is passed into the slice pipeline to make showing the performance differences easier. So far I only have a
[BVH implemented](https://github.com/DanielToby/gpu-slicer/blob/main/slicer/src/spatial_index/no_spatial_index.cpp). A
K-D tree would probably be even faster than the results shown above. This is the data structure that I'm hoping to
accelerate using the GPU.

```C++
class I_SpatialIndex {
public:
    virtual ~I_SpatialIndex() = default;

    //! Constructs the spatial index.
    virtual void build(const std::vector<Triangle3D>& triangles) = 0;

    //! Returns all triangles intersecting zPosition.
    [[nodiscard]] virtual std::vector<Triangle3D> query(float zPosition) const = 0;

    //! The BBox of the index.
    [[nodiscard]] virtual BBox3D AABB() const = 0;
};
```

### The Slicer

See [main.test.cpp](https://github.com/DanielToby/gpu-slicer/blob/main/slicer/src/slicer/main.test.cpp) for the demo taking an stl file and writing SVGs:

```C++

TEST_CASE("Slice 3DBenchy") {
    auto allOperationTimes = timing::LabelToAccumulatedDuration<std::chrono::milliseconds>{};
    const auto triangles = loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");

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

```

See [slicer.cpp](https://github.com/DanielToby/gpu-slicer/blob/main/slicer/src/slicer/slicer.cpp) for a simple breakdown of the core components of a 3D slicer:

```C++
std::vector<Slice> slice(const I_SpatialIndex& mesh, float thickness) {
    std::vector<Slice> result;

    // For timing each slicing operation at each slice height.
    timing::LabelToAccumulatedDuration<std::chrono::microseconds> accumulatedDurations;

    auto time = timing::Clock::now();

    for (const auto& sliceHeight : getSliceHeights(mesh.AABB(), thickness)) {
        auto triangles = mesh.query(sliceHeight);
        timing::timeAndStore(time, "1: query spatial index", accumulatedDurations);

        auto segments = intersect(triangles, sliceHeight);
        timing::timeAndStore(time, "2: intersect triangles", accumulatedDurations);

        auto adjacencyList = getManifoldAdjacencyList(segments);
        timing::timeAndStore(time, "3: build adjacency list", accumulatedDurations);

        auto outlines = getSliceOutlines(adjacencyList);
        timing::timeAndStore(time, "4: get slice outlines", accumulatedDurations);

        auto relativeOutlines = identifyWindings(outlines);
        timing::timeAndStore(time, "5: identify windings", accumulatedDurations);

        auto outlineHierarchy = OutlineHierarchy{std::move(relativeOutlines)};
        timing::timeAndStore(time, "6: get outline hierarchy", accumulatedDurations);

        auto polygons =  outlineHierarchy.getPolygons();
        timing::timeAndStore(time, "7: get polygons", accumulatedDurations);

        result.push_back({{polygons.begin(), polygons.end()}, sliceHeight});
    }

    timing::logTimings("Slice Operations", accumulatedDurations, result.size());

    return result;
}
```

### Dependencies

- [stl_reader](https://github.com/sreiter/stl_reader/tree/6c7615759269a2e6958d04d4d6a4982b6b0da902)
- [catch2](https://github.com/catchorg/Catch2/tree/b3fb4b9feafcd8d91c5cb510a4775143fdbef02f)
