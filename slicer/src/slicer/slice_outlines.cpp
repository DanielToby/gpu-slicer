#include "slicer/slice_outlines.hpp"

#include <numeric>
#include <ranges>
#include <unordered_set>

#include "slicer/bbox.hpp"

namespace slicer {

namespace {

[[nodiscard]] float getDeterminant(const Vec2& p0, const Vec2& p1) {
    return p0.x() * p1.y() - p1.x() * p0.y();
}

std::optional<Vec2> intersect(const Segment2D& line, const Ray2D& ray) {
    const auto s = line.v1 - line.v0;
    const auto rxs = getDeterminant(ray.direction, s);
    if (rxs == 0) {
        return std::nullopt;
    }

    const auto c = line.v0 - ray.p0;
    const auto t = getDeterminant(c, s) / rxs;
    const auto u = getDeterminant(c, ray.direction) / rxs;

    if (0 <= t && 0 <= u && u <= 1) {
        return {line.v0 + (line.v1 - line.v0) * u};
    }
    return std::nullopt;
}

//! Sorts outlines in ascending order by AABB size.
[[nodiscard]] std::vector<SliceOutlineWithWinding> sort(std::vector<SliceOutlineWithWinding>&& outlines) {
    auto result = std::move(outlines);
    std::ranges::sort(outlines, [](const auto& a, const auto& b) {
        return getAABB(a.outline) < getAABB(b.outline);
    });
    return result;
}

//! Returns true if the point is inside the outline.
//! This is done by drawing a vector starting from the point and pointing in an arbitrary direction (right),
//! then counting how many edges the line intersects with. If even, point is outside, else inside.
[[nodiscard]] bool isInside(const Vec2& point, const SliceOutline& outline) {
    const auto ray = Ray2D{point, Vec2{1, 0}};
    auto numIntersections = std::size_t{0};
    for (const auto& segment : getSegments(outline)) {
        if (intersect(segment, ray)) {
            ++numIntersections;
        }
    }
    return numIntersections % 2 == 1;
}

[[nodiscard]] bool isInside(const SliceOutline& maybeInner, const SliceOutline& outer) {
    // It doesn't matter what point we use here, a outline can't be partially inside another.
    return isInside(maybeInner.front(), outer);
}

}

std::vector<Segment2D> getSegments(const SliceOutline& outline) {
    std::vector<Segment2D> result;
    for (auto i = 0; i < outline.size(); ++i) {
        const auto j = (i + 1) % outline.size();
        result.push_back(Segment2D{outline[i], outline[j]});
    }
    return result;
}

std::vector<SliceOutline> getSliceOutlines(const ManifoldAdjacencyList& adjacencyList) {
    using ValueType = ManifoldAdjacencyList::value_type;
    auto vertices = adjacencyList | std::views::transform(&ValueType::first);
    auto unvisited = std::unordered_set<Vec2, Vec2Hash>(vertices.begin(), vertices.end());

    std::vector<SliceOutline> result;
    while (!unvisited.empty()) {
        SliceOutline outline;

        auto start = *unvisited.begin();
        auto current = start;
        std::optional<Vec2> previous = std::nullopt;
        do {
            outline.emplace_back(current);
            unvisited.erase(current);

            const auto& [neighborA, neighborB] = adjacencyList.at(current);
            const auto next = (neighborA == previous) ? neighborB : neighborA;
            previous = current;
            current = next;
        } while (current != start);

        result.push_back(std::move(outline));
    }
    return result;
}

float SliceOutlineWithWinding::calculateArea(const SliceOutline& outline) {
    // The area is defined as 1/2 * the sum of the determinants of the two vectors formed by each point (p0, p1) and the origin.
    // Recall that the determinant corresponds to the area of the parallelogram formed by copying and translating the two vectors.
    // Taking one half of that tells us the area of the triangle formed by those two vectors.
    // If we imagine our outline as drawing a polygon around the origin, the sum of those triangle areas is the area of the polygon.
    // The signedness of each triangle's area allows this to work even if the origin isn't in the polygon at all; outside areas are subtracted.
    auto accumulatedArea = 0.f;
    for (const auto& segment : getSegments(outline)) {
        accumulatedArea += getDeterminant(segment.v0, segment.v1);
    }
    return .5f * accumulatedArea;
}

std::vector<SliceOutlineWithWinding> identifyWindings(const std::vector<SliceOutline>& outlines) {
    std::vector<SliceOutlineWithWinding> result;
    for (const auto& outline : outlines) {
        result.emplace_back(outline);
    }
    return result;
}

bool OutlineHierarchyNode::insert(std::size_t i, std::span<const SliceOutlineWithWinding> sortedOutlines) {
    if (!m_index || isInside(sortedOutlines[i].outline, sortedOutlines[*m_index].outline)) {
        for (auto& child : m_children) {
            if (child.insert(i, sortedOutlines)) {
                return true;
            }
        }
        m_children.emplace_back(i);
        return true;
    }
    return false;
}

void writePolygon(const OutlineHierarchyNode& sourceNode, std::span<const SliceOutlineWithWinding> sourceOutlines, std::vector<Polygon2D>& destination) {
    if (!sourceNode.index()) {
        throw std::runtime_error("Don't call on root.");
    }
    auto outline = sourceOutlines[*sourceNode.index()];
    outline.setWinding(Winding::CCW);

    auto newPolygonIt = destination.insert(destination.end(), Polygon2D{outline.outline});
    for (const auto& child : sourceNode.children()) {
        writeHole(child, sourceOutlines, destination, *newPolygonIt);
    }
}

void writeHole(const OutlineHierarchyNode& sourceNode, std::span<const SliceOutlineWithWinding> sourceOutlines, std::vector<Polygon2D>& destinationRoot, Polygon2D& destinationParent) {
    if (!sourceNode.index()) {
        throw std::runtime_error("Don't call on root.");
    }
    auto outline = sourceOutlines[*sourceNode.index()];
    outline.setWinding(Winding::CW);

    destinationParent.holes.push_back({outline.outline});
    for (const auto& child : sourceNode.children()) {
        writePolygon(child, sourceOutlines, destinationRoot);
    }
}

OutlineHierarchy::OutlineHierarchy(std::vector<SliceOutlineWithWinding>&& outlines) :
    m_sortedOutlines(sort(std::move(outlines))) {
    for (auto i = 0; i < m_sortedOutlines.size(); ++i) {
        if (!m_hierarchy.insert(i, m_sortedOutlines)) {
            throw std::invalid_argument("Failed to insert outline.");
        }
    }
}

std::vector<Polygon2D> OutlineHierarchy::getPolygons() const {
    auto result = std::vector<Polygon2D>{};
    for (const auto& child : m_hierarchy.children()) {
        writePolygon(child, m_sortedOutlines, result);
    }
    return result;
}

}
