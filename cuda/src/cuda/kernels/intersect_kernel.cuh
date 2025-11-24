#pragma once

#include <vector>

namespace cuda::kernels {

struct GPUVec3 {
  float x;
  float y;
  float z;
};

struct GPUTriangle3D {
  GPUVec3 v0;
  GPUVec3 v1;
  GPUVec3 v2;
};

void runIntersectKernel(const std::vector<GPUTriangle3D>& triangles, float zPosition);

}
