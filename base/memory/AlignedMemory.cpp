//
// Created by wang rl on 2022/6/13.
//

#include "base/utils/Bits.h"
#include "base/memory/AlignedMemory.h"

namespace mm {
    void* AlignedAlloc(size_t size, size_t alignment) {
        DCHECK_GT(size, 0U);
        DCHECK(IsPowerOfTwo(alignment));
        DCHECK_EQ(alignment % sizeof(void*), 0U);
        void* ptr = nullptr;
#if defined(_MSC_VER)
        ptr = _aligned_malloc(size, alignment);
#else
        int ret = posix_memalign(&ptr, alignment, size);
        if (ret != 0) {
            DLOG(ERROR) << "posix_memalign() returned with error " << ret;
            ptr = nullptr;
        }
#endif

        // Since aligned allocations may fail for non-memory related reasons, force a
        // crash if we encounter a failed allocation; maintaining consistent behavior
        // with a normal allocation failure.
        if (!ptr) {
            DLOG(ERROR) << "If you crashed here, your aligned allocation is incorrect: "
                        << "size = " << size << ", alignment = " << alignment;
            CHECK(false);
        }

        // Sanity check alignment just to be safe.
        DCHECK(IsAligned(ptr, alignment));
        return ptr;
    }
}
