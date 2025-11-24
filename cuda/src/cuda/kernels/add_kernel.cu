#include "cuda/kernels/add_kernel.cuh"

#include <cuda_runtime.h>


__global__ void addKernel(const int* a, const int* b, int* out, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        out[idx] = a[idx] + b[idx];
    }
}

namespace cuda::kernels {

void runAddKernel(const int* h_a, const int* h_b, int* h_out, int n) {
    int *d_a = nullptr, *d_b = nullptr, *d_out = nullptr;

    // Allocate device memory
    cudaMalloc(&d_a, n * sizeof(int));
    cudaMalloc(&d_b, n * sizeof(int));
    cudaMalloc(&d_out, n * sizeof(int));

    // Copy inputs to device
    cudaMemcpy(d_a, h_a, n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, n * sizeof(int), cudaMemcpyHostToDevice);

    // Launch kernel
    int blockSize = 256;
    int numBlocks = (n + blockSize - 1) / blockSize;
    addKernel<<<numBlocks, blockSize>>>(d_a, d_b, d_out, n);

    cudaDeviceSynchronize();

    // Copy result back
    cudaMemcpy(h_out, d_out, n * sizeof(int), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_out);
}

}
