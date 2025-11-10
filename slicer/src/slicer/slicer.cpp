#include "slicer/slicer.hpp"

namespace slicer {

std::vector<double> getSliceHeights(const BBox3D& volume, float thickness_mm) {
    std::vector<double> result;
    const auto bottom = volume.min.z;
    result.push_back(volume.min.z);

    auto current = bottom + thickness_mm;
    while (current <= volume.max.z) {
        result.push_back(current);
        current += thickness_mm;
    }

    return result;
}

}