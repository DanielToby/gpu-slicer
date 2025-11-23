#include "add_kernel.cuh"

#include <cuda_runtime.h>

#include <stdio.h>

__global__ void addKernel(const int* a, const int* b, int* out, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        out[idx] = a[idx] + b[idx];
    }
}

#define CUDA_CHECK(x)                                           \
do {                                                        \
cudaError_t err = (x);                                  \
if (err != cudaSuccess) {                               \
printf("CUDA error %s at %s:%d\n",                  \
cudaGetErrorString(err), __FILE__, __LINE__);\
}                                                       \
} while (0)


void runAddKernel(const int* h_a, const int* h_b, int* h_out, int n) {
    int *d_a = nullptr, *d_b = nullptr, *d_out = nullptr;

    // Allocate device memory
    CUDA_CHECK(cudaMalloc(&d_a, n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_b, n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_out, n * sizeof(int)));

    // Copy inputs to device
    CUDA_CHECK(cudaMemcpy(d_a, h_a, n * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_b, h_b, n * sizeof(int), cudaMemcpyHostToDevice));

    // Launch kernel
    int blockSize = 256;
    int numBlocks = (n + blockSize - 1) / blockSize;
    addKernel<<<numBlocks, blockSize>>>(d_a, d_b, d_out, n);

    CUDA_CHECK(cudaDeviceSynchronize());

    // Copy result back
    CUDA_CHECK(cudaMemcpy(h_out, d_out, n * sizeof(int), cudaMemcpyDeviceToHost));

    // Free device memory
    CUDA_CHECK(cudaFree(d_a));
    CUDA_CHECK(cudaFree(d_b));
    CUDA_CHECK(cudaFree(d_out));
}
