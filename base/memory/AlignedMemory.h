//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_ALIGNED_MEMORY_H
#define MULTIMEDIA_ALIGNED_MEMORY_H

#if defined(_MSC_VER)
#include <malloc.h>
#else
#include <cstdlib>
#endif

#include <glog/logging.h>
#include "base/utils/Bits.h"

namespace mm {
    // This can be replaced with std::aligned_alloc when we have C++17.
    // Caveat: std::aligned_alloc requires the size parameter be an integral
    // multiple of alignment.
    void* AlignedAlloc(size_t size, size_t alignment);

    inline void AlignedFree(void* ptr) {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }

    struct AlignedFreeDeleter {
        inline void operator()(void* ptr) const {
            AlignedFree(ptr);
        }
    };

    inline bool IsAligned(uintptr_t val, size_t alignment) {
        DCHECK(mm::IsPowerOfTwo(alignment)) << alignment << " is not a power of 2";
        return (val & (alignment - 1)) == 0;
    }

    inline bool IsAligned(const void* val, size_t alignment) {
        return IsAligned(reinterpret_cast<uintptr_t>(val), alignment);
    }
}

#endif //MULTIMEDIA_ALIGNED_MEMORY_H
