//
// Created by wang rl on 2022/6/9.
//

#ifndef MULTIMEDIA_UTILITIES_H
#define MULTIMEDIA_UTILITIES_H

#include <cstdint>

namespace mm {
    /**
     * Convert an array of floats to an array of 16-bit integers.
     *
     * @param source the input array.
     * @param destination the output array.
     * @param numSamples the number of values to convert.
     */
    void convertFloatToPcm16(const float *source, int16_t *destination, int32_t numSamples);

    /**
     * Convert an array of 16-bit integers to an array of floats.
     *
     * @param source the input array.
     * @param destination the output array.
     * @param numSamples the number of values to convert.
     */
    void convertPcm16ToFloat(const int16_t *source, float *destination, int32_t numSamples);
}

#endif //MULTIMEDIA_UTILITIES_H
