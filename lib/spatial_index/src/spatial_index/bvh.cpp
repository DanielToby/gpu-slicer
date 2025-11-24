#include "spatial_index/bvh.hpp"

#include <stdexcept>

#include "spatial_index/variant.hpp"

namespace slicer {

namespace {

using detail::BVHTriangle;

[[nodiscard]] Vec3 getCentroid(const Triangle3D& triangle) {
    const auto xAverage = (triangle.v0.x() + triangle.v1.x() + triangle.v2.x()) / 3;
    const auto yAverage = (triangle.v0.y() + triangle.v1.y() + triangle.v2.y()) / 3;
    const auto zAverage = (triangle.v0.z() + triangle.v1.z() + triangle.v2.z()) / 3;
    return {xAverage, yAverage, zAverage};
}

[[nodiscard]] BBox3D getAABB(std::span<const BVHTriangle> triangles) {
    BBox3D result;
    for (const auto& triangle : triangles) {
        result.extend(triangle.bbox);
    }
    return result;
}

double getSurfaceArea(const BBox3D& bbox) {
    const auto w = bbox.max.x() - bbox.min.x();
    const auto h = bbox.max.y() - bbox.min.y();
    const auto d = bbox.max.z() - bbox.min.z();
    return 2 * (w * h + h * d + w * d);
}

constexpr int MAX_PRIMITIVES_PER_BVH_LEAF = 8;

namespace SurfaceAreaHeuristic {

const double traversalCost = 1;
const double intersectionCost = 2;

double calculate(std::span<const BVHTriangle> left, std::span<const BVHTriangle> right, double parentSurfaceArea) {
    auto leftSurfaceArea = getSurfaceArea(getAABB(left));
    auto rightSurfaceArea = getSurfaceArea(getAABB(right));
    auto leftCost = (leftSurfaceArea / parentSurfaceArea) * left.size();
    auto rightCost = (rightSurfaceArea / parentSurfaceArea) * right.size();
    return traversalCost + intersectionCost * (leftCost + rightCost);
}

struct BVHSplitCandidate {
    std::span<const BVHTriangle> left;
    std::span<const BVHTriangle> right;
    double cost;
};

//! Finds the split candidate that minimizes SAH cost.
[[nodiscard]] BVHSplitCandidate getBestCandidate(std::span<const BVHTriangle> zSortedTriangles) {
    auto splitPoints = detail::getBalancedBinarySplitPoints(zSortedTriangles.size(), MAX_PRIMITIVES_PER_BVH_LEAF);
    if (splitPoints.empty()) {
        throw std::runtime_error("Expected non-zero BVH split points.");
    }

    auto parentSurfaceArea = getSurfaceArea(getAABB(zSortedTriangles));
    auto candidates = splitPoints | std::views::transform([&](std::size_t splitPoint) {
                          auto left = zSortedTriangles.subspan(0, splitPoint);
                          auto right = zSortedTriangles.subspan(splitPoint);
                          auto cost = SurfaceAreaHeuristic::calculate(left, right, parentSurfaceArea);
                          return BVHSplitCandidate{left, right, cost};
                      });

    if (auto it = std::ranges::min_element(candidates, {}, &BVHSplitCandidate::cost); it != candidates.end()) {
        return *it;
    }
    throw std::runtime_error("Expected non-zero BVH split candidates.");
}

}

[[nodiscard]] std::vector<Triangle3D> getTriangles(std::span<const BVHTriangle> bvhTriangles) {
    auto result = bvhTriangles | std::views::transform(&BVHTriangle::triangle);
    return {result.begin(), result.end()};
}

[[nodiscard]] bool intersects(const BBox3D& bbox, float zPosition) {
    return bbox.min.z() <= zPosition && bbox.max.z() >= zPosition;
}

void addLeafTriangles(const detail::BVHLeaf& leaf, std::vector<Triangle3D>& triangles, float zPosition) {
    for (const auto& triangle : leaf.primitives) {
        if (intersects(triangle, zPosition)) {
            triangles.push_back(triangle);
        }
    }
}

void addNodeTriangles(const detail::BVHNode::NodeT& node, std::vector<Triangle3D>& triangles, float zPosition) {
    std::visit(overloaded{
                   [&](const detail::BVHLeaf& leaf) {
                       addLeafTriangles(leaf, triangles, zPosition);
                   },
                   [&](const std::unique_ptr<detail::BVHNode>& innerNode) {
                       if (intersects(innerNode->bbox, zPosition)) {
                           addNodeTriangles(innerNode->left, triangles, zPosition);
                           addNodeTriangles(innerNode->right, triangles, zPosition);
                       }
                   }},
               node);
}

}

std::size_t detail::getNumNodes(const BVHNode::NodeT& node) {
    auto numNodesInner = [](const std::unique_ptr<BVHNode>& innerNode) -> std::size_t {
        return 1 + getNumNodes(innerNode->left) + getNumNodes(innerNode->right);
    };
    auto numNodesLeaf = [](const BVHLeaf&) -> std::size_t {
        return 1;
    };
    return std::visit(overloaded{numNodesInner, numNodesLeaf}, node);
}

std::vector<BVHTriangle> detail::getZSortedBVHTriangles(std::span<const Triangle3D> triangles) {
    auto asBvhTriangles = triangles | std::views::transform([](const auto& triangle) {
                              return BVHTriangle{triangle, getCentroid(triangle), getAABB(triangle)};
                          });
    auto result = std::vector<BVHTriangle>{asBvhTriangles.begin(), asBvhTriangles.end()};
    std::ranges::sort(result, [](const auto& left, const auto& right) {
        return left.centroid.z() < right.centroid.z();
    });
    return result;
}

std::vector<std::size_t> detail::getBalancedBinarySplitPoints(std::size_t numItems, std::size_t minItemsPerSide) {
    std::vector<std::size_t> result;
    if (numItems < minItemsPerSide * 2) {
        return result;
    }
    auto end = minItemsPerSide > 0 ? (numItems - minItemsPerSide + 1) : numItems;
    for (auto i = minItemsPerSide; i < end; ++i) {
        result.emplace_back(i);
    }
    return result;
}

detail::BVHNode::NodeT detail::getBVHNode(std::span<const BVHTriangle> zSortedTriangles, const ConstructionStrategy& strategy) {
    // Leaf
    if (zSortedTriangles.size() <= MAX_PRIMITIVES_PER_BVH_LEAF) {
        return BVHLeaf{getTriangles(zSortedTriangles)};
    }

    auto makeInternalNode = [&](const auto& leftSpan, const auto& rightSpan) {
        auto left = getBVHNode(leftSpan, strategy);
        auto right = getBVHNode(rightSpan, strategy);

        auto bbox = getAABB(leftSpan);
        bbox.extend(getAABB(rightSpan));

        return std::make_unique<BVHNode>(std::move(left), std::move(right), bbox);
    };

    // Uneven Split: one candidate is made by balancing the available primitives. This is probably suboptimal.
    if (zSortedTriangles.size() < MAX_PRIMITIVES_PER_BVH_LEAF * 2) {
        auto midPoint = zSortedTriangles.size() / 2;
        return makeInternalNode(zSortedTriangles.subspan(0, midPoint), zSortedTriangles.subspan(midPoint));
    }

    // N even splits. Select best candidate using SAH.
    switch (strategy) {
    case ConstructionStrategy::BinarySplit: {
        auto midPoint = zSortedTriangles.size() / 2;
        return makeInternalNode(zSortedTriangles.subspan(0, midPoint), zSortedTriangles.subspan(midPoint));
    }
    case ConstructionStrategy::SurfaceAreaHeuristic: {
        auto bestCandidate = SurfaceAreaHeuristic::getBestCandidate(zSortedTriangles);
        return makeInternalNode(bestCandidate.left, bestCandidate.right);
    }
    default:
        throw std::runtime_error("Unknown construction strategy.");
    }
}

void detail::addTriangles(const BVHNode& node, std::vector<Triangle3D>& triangles, float zPosition) {
    addNodeTriangles(node.left, triangles, zPosition);
    addNodeTriangles(node.right, triangles, zPosition);
}

void BVH::build(const std::vector<Triangle3D>& triangles) {
    auto zSortedTriangles = detail::getZSortedBVHTriangles(triangles);
    auto root = getBVHNode(zSortedTriangles, m_strategy);
    if (std::holds_alternative<detail::BVHLeaf>(root)) {
        throw std::runtime_error("Too few triangles.");
    }
    m_root = std::move(std::get<std::unique_ptr<detail::BVHNode>>(root));
}

std::vector<Triangle3D> BVH::query(float zPosition) const {
    if (!m_root) {
        throw std::runtime_error("Uninitialized. Call build(...) first.");
    }

    std::vector<Triangle3D> result;
    addTriangles(*m_root, result, zPosition);
    return result;

}

BBox3D BVH::AABB() const {
    if (!m_root) {
        throw std::runtime_error("Uninitialized. Call build(...) first.");
    }
    return m_root->bbox;
}

std::size_t BVH::getNumNodes() const {
    if (!m_root) {
        return 0;
    }
    return 1 + detail::getNumNodes(m_root->left) + detail::getNumNodes(m_root->right);
}

}
