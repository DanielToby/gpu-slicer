#pragma once

#include <string>
#include <vector>

#include "geometry/geometry.hpp"

namespace slicer {

std::vector<Triangle3D> loadStl(const std::string& fileName) noexcept(false);

}