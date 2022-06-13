//
// Created by wang rl on 2022/6/13.
//

// Contains limit definition constants for the media subsystem.

#ifndef MULTIMEDIA_LIMITS_H
#define MULTIMEDIA_LIMITS_H

namespace mm {
    // Maximum possible dimension (with or height) for any video.
    constexpr int kMaxDimension = (1 << 15) - 1;    // 32767

    // Maximum possible canvas size (width multiplied by height) for any video.
    constexpr int kMaxCanvas = (1 << (14 * 2));  // 16384 x 16384

    // Total number of video frames which are populating in the pipeline.
    constexpr int kMaxVideoFrames = 4;

    // The following limits are used by AudioParameters::IsValid().
    //
    // A few notes on sample rates of common formats:
    //   - AAC files are limited to 96 kHz.
    //   - MP3 files are limited to 48 kHz.
    //   - Vorbis used to be limited to 96 kHz, but no longer has that
    //     restriction.
    //   - Most PC audio hardware is limited to 192 kHz, some specialized DAC
    //     devices will use 768 kHz though.
    constexpr int kMaxSampleRate = 768000;
    constexpr int kMinSampleRate = 3000;
    constexpr int kMaxChannels = 32;
    constexpr int kMaxBytesPerSample = 4;

}

#endif //MULTIMEDIA_LIMITS_H
