//
// Created by wang rl on 2022/6/9.
//

#include "common/Utilities.h"

namespace mm {
    constexpr float kScaleI16ToFloat = (1.0f / 32768.0f);

    void convertFloatToPcm16(const float *source, int16_t *destination, int32_t numSamples) {
        for (int i = 0; i < numSamples; i++) {
            float fval = source[i];
            fval += 1.0; // to avoid discontinuity at 0.0 caused by truncation
            fval *= 32768.0f;
            auto sample = static_cast<int32_t>(fval);
            // clip to 16-bit range
            if (sample < 0) sample = 0;
            else if (sample > 0x0FFFF) sample = 0x0FFFF;
            sample -= 32768; // center at zero
            destination[i] = static_cast<int16_t>(sample);
        }
    }

    void convertPcm16ToFloat(const int16_t *source, float *destination, int32_t numSamples) {
        for (int i = 0; i < numSamples; i++) {
            destination[i] = source[i] * kScaleI16ToFloat;
        }
    }
}