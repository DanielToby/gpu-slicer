#pragma once

#include "slicer/adjacency_list.hpp"

namespace slicer {

//! After slicing, connecting segments produces outlines of unknown winding (interior vs exterior).
using SliceOutline = std::vector<Vec2>;

[[nodiscard]] std::vector<Segment2D> getSegments(const SliceOutline& outline);

//! Produces a collection of outlines by following adjacent points.
[[nodiscard]] std::vector<SliceOutline> getSliceOutlines(const ManifoldAdjacencyList& adjacencyList);

//! The windings read from slice outlines are relative. We don't know whether they correspond to CCW or CW yet.
enum class RelativeWinding { Negative = 0, Positive = 1 };

//! Inspects the sign of the outlines area to identify a relative winding.
struct SliceOutlineWithRelativeWinding {
    SliceOutline outline;
    RelativeWinding relativeWinding = RelativeWinding::Negative;
};

//! Assigns a relative winding to each outline.
[[nodiscard]] std::vector<SliceOutlineWithRelativeWinding> identifyWindings(const std::vector<SliceOutline>& outlines);

//! After outlines are sorted ascending by AABB size, we don't change their position in the flat container.
//! We use these indices to refer to them, and trust that the flat container lives at least as long as these.
class OutlineHierarchyNode {
public:
    OutlineHierarchyNode() = default;
    explicit OutlineHierarchyNode(std::size_t index) : m_index(index) {}

    //! If the outline is inside this, inserts it into the smallest containing outline.
    [[nodiscard]] bool insert(std::size_t i, std::span<const SliceOutlineWithRelativeWinding> sortedOutlines);

    //! Only the root node has nullopt index.
    [[nodiscard]] std::optional<std::size_t> index() const { return m_index; }
    [[nodiscard]] std::vector<OutlineHierarchyNode> children() const { return m_children; }

private:
    std::optional<std::size_t> m_index;
    std::vector<OutlineHierarchyNode> m_children;
};

//! Determines whether outlines are added as new polygons, or holes within existing polygons.
enum class WindingFlag {
    Shell = 0, // CCW
    Hole = 1   // Hole
};

enum class EnforceWinding : bool { no, yes };

//! Writes the polygon using the geometry in sourceOutlines at the index from sourceNode.
void writePolygon(
    const OutlineHierarchyNode& sourceNode,
    std::span<const SliceOutlineWithRelativeWinding> sourceOutlines,
    std::vector<Polygon2D>& destination,
    EnforceWinding = EnforceWinding::yes);

//! Writes the hole using the geometry in sourceOutlines at the index from sourceNode.
void writeHole(const OutlineHierarchyNode& sourceNode,
               std::span<const SliceOutlineWithRelativeWinding> sourceOutlines,
               std::vector<Polygon2D>& destinationRoot,
               Polygon2D& destinationParent,
               EnforceWinding = EnforceWinding::yes);

//! Attaches the lifetime of the sorted outlines to the hierarchy built in the c'tor.
class OutlineHierarchy {
public:
    explicit OutlineHierarchy(std::vector<SliceOutlineWithRelativeWinding>&& outlines);

    [[nodiscard]] std::vector<Polygon2D> getPolygons() const;

private:
    std::vector<SliceOutlineWithRelativeWinding> m_sortedOutlines;
    OutlineHierarchyNode m_hierarchy;
};


}
