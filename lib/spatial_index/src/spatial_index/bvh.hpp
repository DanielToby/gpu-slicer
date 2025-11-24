#pragma once

#include <memory>
#include <variant>

#include "spatial_index/spatial_index.hpp"

namespace slicer {

//! The different strategies supported for constructing the BVH.
enum class ConstructionStrategy {
    BinarySplit,         // Sorts triangles by their centroids z-position, then splits them evenly.
    SurfaceAreaHeuristic // Sorts triangles by their centroids z-position, then chooses the split with lowest SAH cost.
};

namespace detail {

//! MAX_PRIMITIVES_PER_BVH_LEAF is implementation defined.
struct BVHLeaf {
    std::vector<Triangle3D> primitives;
};

//! A node is either two more nodes or a leaf.
struct BVHNode;
struct BVHNode {
    // Unique pointer is used because we need forward declaration.
    using NodeT = std::variant<std::unique_ptr<BVHNode>, BVHLeaf>;

    BVHNode(NodeT&& left, NodeT&& right, const BBox3D& bbox) :
        left(std::move(left)),
        right(std::move(right)),
        bbox(bbox) {}

    // This is a binary tree.
    NodeT left;
    NodeT right;

    // To reduce recalculations
    BBox3D bbox;
};

//! Returns the number of nodes in the bvh, including this one (the root).
[[nodiscard]] std::size_t getNumNodes(const BVHNode::NodeT& node);

//! Attaches some extra metadata to the triangle, to avoid recalculations.
struct BVHTriangle {
    Triangle3D triangle;
    Vec3 centroid;
    BBox3D bbox;
};

//! Creates BVHTriangles from the provided triangles, then sorts them in ascending order by the z-location of their centroids.
[[nodiscard]] std::vector<BVHTriangle> getZSortedBVHTriangles(std::span<const Triangle3D> triangles);

//! Returns every split point for a range of size `numItems`. Returns no split points if minItemsPerSide is not possible.
[[nodiscard]] std::vector<std::size_t> getBalancedBinarySplitPoints(std::size_t numItems, std::size_t minItemsPerSide);

//! Returns a node containing all the provided triangles.
//! Returns a leaf node if the number of triangles is less than MAX_TRIANGLES_PER_LEAF.
//! Otherwise, returns a node corresponding to the candidate with the lowest cost, based on the ConstructionStrategy.
[[nodiscard]] BVHNode::NodeT getBVHNode(std::span<const BVHTriangle> zSortedTriangles, const ConstructionStrategy& strategy);

//! Adds any triangles present in `node` that intersect `zPosition`.
void addTriangles(const BVHNode& node, std::vector<Triangle3D>& triangles, float zPosition);

}

//! Divides primitives into two distinct groups based on a heuristic until a maximum number of primitives per leaf is achieved.
//! https://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
class BVH : public I_SpatialIndex {
public:
    explicit BVH(ConstructionStrategy strategy) : m_strategy{strategy} {}

    //! Constructs the spatial index.
    void build(const std::vector<Triangle3D>& triangles) override;

    //! Returns all triangles intersecting zPosition.
    [[nodiscard]] std::vector<Triangle3D> query(float zPosition) const override;

    //! The BBox of the index.
    [[nodiscard]] BBox3D AABB() const override;

    //! Returns the number of nodes in the BVH.
    [[nodiscard]] std::size_t getNumNodes() const;

private:
    ConstructionStrategy m_strategy;
    // Null until `build` is called with more than two triangles.
    std::unique_ptr<detail::BVHNode> m_root;
};

}
