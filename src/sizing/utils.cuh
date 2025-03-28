#ifndef __UTILS_CUH
#define __UTILS_CUH

#include <assert.h>

#include <cuda_runtime.h>

#include "scalar_field.hpp"

namespace cuda_utils
{
    __device__ __inline__ unsigned int tidx()
    {
        return blockIdx.x * blockDim.x + threadIdx.x;
    }

    __device__ __inline__ unsigned int tidy()
    {
        return blockIdx.y * blockDim.y + threadIdx.y;
    }

    __device__ __inline__ unsigned int tidz()
    {
        return blockIdx.z * blockDim.z + threadIdx.z;
    }

    namespace pitched
    {
        template <typename T>
        __device__ T get(const T* pointer, size_t pitch, unsigned int x, unsigned int y, unsigned int z)
        {
            // asserts will not print to stderr upon synchronization on MacOS
            // assert((!(x < 0 && y < 0 && z < 0)) && "memory region out of bounds");
            // assert((!(x >= matrix.width || y >= matrix.height || z >= matrix.depth)) && "memory region out of bounds");

            T *row = (T*) pointer + z * pointer->ysize + y * pointer->pitch;
            return row[x];
        }

        template <typename T>
        __device__ void set(T* pointer, size_t pitch, unsigned int x, unsigned int y, unsigned int z, T value)
        {
            // asserts will not print to stderr upon synchronization on MacOS
            // assert((!(x < 0 && y < 0 && z < 0)) && "memory region out of bounds");
            // assert((!(x >= matrix.width || y >= matrix.height || z >= matrix.depth)) && "memory region out of bounds");

            T *row = (T*) pointer->ptr + z * pointer->ysize + y * pointer->pitch;
            row[x] = value;
        }
    };
};

#endif // __UTILS_CUH
