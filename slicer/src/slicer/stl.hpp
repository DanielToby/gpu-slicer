#pragma once

#include <vector>

#include "slicer/mesh.hpp"

namespace slicer {

Mesh loadStl(const std::string& fileName) noexcept(false);

}