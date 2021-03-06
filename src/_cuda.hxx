#pragma once
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cuda_runtime.h>
#include <cuda_profiler_api.h>
#include "_main.hxx"

using std::fprintf;
using std::exit;




// LAUNCH CONFIG
// -------------

// Limits
#define BLOCK_LIMIT 1024
#define GRID_LIMIT  65535

// For map-like operations
#define BLOCK_DIM_M 256
#define GRID_DIM_M  GRID_LIMIT

// For reduce-like operations
template <class T>
constexpr int BLOCK_DIM_R() noexcept { return sizeof(T)<=4? 128:256; }
template <class T>
constexpr int GRID_DIM_R() noexcept { return 1024; }




// TRY
// ---
// Log error if CUDA function call fails.

#ifndef TRY_CUDA
void tryCuda(cudaError err, const char* exp, const char* func, int line, const char* file) {
  if (err == cudaSuccess) return;
  fprintf(stderr,
    "%s: %s\n"
    "  in expression %s\n"
    "  at %s:%d in %s\n",
    cudaGetErrorName(err), cudaGetErrorString(err), exp, func, line, file);
  exit(err);
}

#define TRY_CUDA(exp) tryCuda(exp, #exp, __func__, __LINE__, __FILE__)
#endif

#ifndef TRY
#define TRY(exp) TRY_CUDA(exp)
#endif




// DEFINE
// ------
// Define thread, block variables.

#ifndef DEFINE_CUDA
#define DEFINE_CUDA(t, b, B, G) \
  const int t = threadIdx.x; \
  const int b = blockIdx.x; \
  const int B = blockDim.x; \
  const int G = gridDim.x;
#define DEFINE_CUDA2D(tx, ty, bx, by, BX, BY, GX, GY) \
  const int tx = threadIdx.x; \
  const int ty = threadIdx.y; \
  const int bx = blockIdx.x; \
  const int by = blockIdx.y; \
  const int BX = blockDim.x; \
  const int BY = blockDim.y; \
  const int GX = gridDim.x;  \
  const int GY = gridDim.y;
#endif

#ifndef DEFINE
#define DEFINE(t, b, B, G) \
  DEFINE_CUDA(t, b, B, G)
#define DEFINE2D(tx, ty, bx, by, BX, BY, GX, GY) \
  DEFINE_CUDA2D(tx, ty, bx, by, BX, BY, GX, GY)
#endif




// UNUSED
// ------
// Mark CUDA kernel variables as unused.

template <class T>
__device__ void unusedCuda(T&&) {}

#ifndef UNUSED_CUDA
#define UNUSED_CUDA(x) unusedCuda(x)
#endif

#ifndef UNUSED
#define UNUSED UNUSED_CUDA
#endif




// REDUCE
// ------

template <class T>
int reduceSizeCu(int N) {
  int B = BLOCK_DIM_R<T>();
  int G = min(ceilDiv(N, B), GRID_DIM_R<T>());
  return G;
}
