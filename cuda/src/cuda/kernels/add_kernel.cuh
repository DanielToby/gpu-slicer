#pragma once


namespace cuda::kernels {

void runAddKernel(const int* h_a, const int* h_b, int* h_c, int N);

}