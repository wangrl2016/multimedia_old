//
// Created by wang rl on 2022/6/13.
//

#include <glog/logging.h>
#include "media/base/AudioBus.h"
#include "media/base/Limits.h"

namespace mm {
    static bool IsAligned(void* ptr) {
        return (reinterpret_cast<uintptr_t>(ptr) &
                (AudioBus::kChannelAlignment - 1) == 0U);
    }

    // In order to guarantee that the memory block for each channel starts at an
    // aligned address when splitting a contiguous block of memory into one block
    // per channel, we may have to make these blocks larger than otherwise needed.
    // We do this by allocating space for potentially more frames than requested.
    // This method returns the required size for the contiguous memory block
    // in bytes and outputs the adjusted number of frames via |outAlignedFrames|.
    static int CalculateMemorySizeInternal(int channels,
                                           int frames,
                                           int* outAlignedFrames) {
        // Since our internal sample format is float, we can guarantee the alignment
        // by making the number of frames an integer multiple of
        // AudioBus::kChannelAlignment / sizeof(float).
        int alignedFrames =
                ((frames * sizeof(float) + AudioBus::kChannelAlignment - 1) &
                 ~(AudioBus::kChannelAlignment - 1)) / sizeof(float);

        if (outAlignedFrames)
            *outAlignedFrames = alignedFrames;

        return sizeof(float) * channels * alignedFrames;
    }

    static void ValidateConfig(int channels, int frames) {
        CHECK_GT(frames, 0);
        CHECK_GT(channels, 0);
        CHECK_LE(channels, static_cast<int>(kMaxChannels));
    }

    void AudioBus::CheckOverflow(int startFrame, int frames, int totalFrames) {
        CHECK_GE(startFrame, 0);
        CHECK_GE(frames, 0);
        CHECK_GT(totalFrames, 0);
        int sum = startFrame + frames;
        CHECK_LE(sum, totalFrames);
        CHECK_GE(sum, 0);
    }

    AudioBus::AudioBus(int channels, int frames)
            : mFrames(frames) {
        ValidateConfig(channels, mFrames);

        int alignedFrames = 0;
        int size = CalculateMemorySizeInternal(channels, frames, &alignedFrames);

        mData.reset(static_cast<float*>(AlignedAlloc(
                size, AudioBus::kChannelAlignment)));
    }
}
