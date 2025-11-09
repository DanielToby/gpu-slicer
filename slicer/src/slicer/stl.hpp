#pragma once

#include <vector>

#include "slicer/fwd.hpp"

namespace slicer {

std::vector<Triangle> loadStl(const std::string& fileName) noexcept(false);

}