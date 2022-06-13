//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_BITS_H
#define MULTIMEDIA_BITS_H

#include <glog/logging.h>
#include <type_traits>

namespace mm {
    // Returns true iff |value| is a power of 2.
    // When C++20 is available, replace with std::has_single_bit().
    template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
    constexpr bool IsPowerOfTwo(T value) {
        // From "Hacker's Delight": Section 2.1 Manipulating Rightmost Bits.
        //
        // Only positive integers with a single bit set are powers of two. If only one
        // bit is set in x (e.g. 0b00000100000000) then |x-1| will have that bit set
        // to zero and all bits to its right set to 1 (e.g. 0b00000011111111). Hence
        // |x & (x-1)| is 0 iff x is a power of two.
        return value > 0 && (value & (value - 1)) == 0;
    }

    // Round down |size| to a multiple of alignment, which must be a power of two.
    inline constexpr size_t AlignDown(size_t size, size_t alignment) {
        DCHECK(IsPowerOfTwo(alignment));
        return size & ~(alignment - 1);
    }

    // Move |ptr| back to the previous multiple of alignment, which must be a power
    // of two. Defined for types where sizeof(T) is one byte.
    template <typename T, typename = typename std::enable_if<sizeof(T) == 1>::type>
    inline T* AlignDown(T* ptr, size_t alignment) {
        return reinterpret_cast<T*>(
                AlignDown(reinterpret_cast<size_t>(ptr), alignment));
    }

    // Round up |size| to a multiple of alignment, which must be a power of two.
    inline constexpr size_t AlignUp(size_t size, size_t alignment) {
        DCHECK(IsPowerOfTwo(alignment));
        return (size + alignment - 1) & ~(alignment - 1);
    }

    // Advance |ptr| to the next multiple of alignment, which must be a power of
    // two. Defined for types where sizeof(T) is one byte.
    template <typename T, typename = typename std::enable_if<sizeof(T) == 1>::type>
    inline T* AlignUp(T* ptr, size_t alignment) {
        return reinterpret_cast<T*>(
                AlignUp(reinterpret_cast<size_t>(ptr), alignment));
    }
}

#endif //MULTIMEDIA_BITS_H
