// matrix_multiply.cu
#include <iostream>
#include <vector>
// CUDA runtime
// #include <cuda_runtime.h>
// #include <cuda_profiler_api.h>

__global__ float elementwise_multiply(float* A, float* B, float* C, int size) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < size) {
        C[idx] = A[idx] * B[idx];
    }
}

void multiply_matrices(float* A, float* B, float* C, int size) {
    int threads_per_block = 256;
    int num_blocks = (size + threads_per_block - 1) / threads_per_block;
    elementwise_multiply<<<num_blocks, threads_per_block>>>(A, B, C, size);
    cudaDeviceSynchronize();
}