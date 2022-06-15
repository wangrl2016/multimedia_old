//
// Created by WangRuiLing on 2022/6/15.
//

#ifndef MULTIMEDIA_AUDIO_SAMPLE_TYPES_H
#define MULTIMEDIA_AUDIO_SAMPLE_TYPES_H

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace mm {
    template<typename SampleType>
    class FloatSampleTypeTraits {
        static_assert(std::is_floating_point<SampleType>::value,
                      "Template is only valid for float types.");

    public:
        using ValueType = SampleType;

        static constexpr SampleType kMinValue = -1.0f;
        static constexpr SampleType kMaxValue = +1.0f;
        static constexpr SampleType kZeroPointValue = 0.0f;

        static SampleType FromFloat(float sourceValue) {
            return From<float>(sourceValue);
        }

        static float ToFloat(SampleType sourceValue) {
            return To<float>(sourceValue);
        }

        static SampleType FromDouble(double sourceValue) {
            return From<double>(sourceValue);
        }

        static double ToDouble(SampleType sourceValue) {
            return To<double>(sourceValue);
        }

    private:
        template<typename FloatType>
        static SampleType From(FloatType sourceValue) {
            // Apply clipping (aka. clamping). These values are frequently sent to OS
            // level drivers that may not properly handle these values.
            if (sourceValue < kMinValue)
                return kMinValue;
            if (sourceValue >= kMaxValue)
                return kMaxValue;
            return static_cast<SampleType>(sourceValue);
        }

        template<typename FloatType>
        static FloatType To(SampleType sourceValue) {
            return static_cast<FloatType>(sourceValue);
        }
    };

    // Similar to above, but does not apply clipping.
    template<typename SampleType>
    class FloatSampleTypeTraitsNoClip {
        static_assert(std::is_floating_point<SampleType>::value,
                      "Template is only valid for float types.");

    public:
        using ValueType = SampleType;

        static constexpr SampleType kMinValue = -1.0f;
        static constexpr SampleType kMaxValue = +1.0f;
        static constexpr SampleType kZeroPointValue = 0.0f;

        static SampleType FromFloat(float source_value) {
            return From<float>(source_value);
        }

        static float ToFloat(SampleType source_value) {
            return To<float>(source_value);
        }

        static SampleType FromDouble(double source_value) {
            return From<double>(source_value);
        }

        static double ToDouble(SampleType source_value) {
            return To<double>(source_value);
        }

    private:
        template<typename FloatType>
        static SampleType From(FloatType source_value) {
            return static_cast<SampleType>(source_value);
        }

        template<typename FloatType>
        static FloatType To(SampleType source_value) {
            return static_cast<FloatType>(source_value);
        }
    };

    // For uint8_t, int16_t, int32_t...
    // See also the aliases for commonly used types at the bottom of this file.
    template<typename SampleType>
    class FixedSampleTypeTraits {
        static_assert(std::numeric_limits<SampleType>::is_integer,
                "Template is only valid for integer types.");

    public:
        using ValueType = SampleType;
    };
}

#endif //MULTIMEDIA_AUDIO_SAMPLE_TYPES_H
