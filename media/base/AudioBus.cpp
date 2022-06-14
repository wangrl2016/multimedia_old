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

    std::unique_ptr<AudioBus> AudioBus::Create(int channels, int frames) {
        return std::unique_ptr<AudioBus>(new AudioBus(channels, frames));
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

    void AudioBus::zero() {
        zeroFrames(mFrames);
    }

    void AudioBus::zeroFrames(int frames) {
        zeroFramesPartial(0, frames);
    }

    void AudioBus::zeroFramesPartial(int startFrame, int frames) {
        CheckOverflow(startFrame, frames, mFrames);
        if (frames <= 0)
            return;

        for (size_t i = 0; i < mChannelData.size(); i++) {
            memset(mChannelData[i] + startFrame, 0,
                   frames * sizeof(*mChannelData[i]));
        }
    }

    bool AudioBus::areFramesZero() const {
        for (size_t i = 0; i < mChannelData.size(); i++) {
            for (int j = 0; j < mFrames; j++) {
                if (mChannelData[i][j])
                    return false;
            }
        }
        return true;
    }

    void AudioBus::scale(float volume) {
        if (volume > 0 && volume != 1) {
            for (int i = 0; i < channels(); i++) {
                // 可以使用硬件加速
                for (int j = 0; j < mFrames; j++) {
                    mChannelData[i][j] = mChannelData[i][j] * volume;
                }
            }
        } else if (volume == 0) {
            zero();
        }
    }

    AudioBus::AudioBus(int channels, int frames)
            : mFrames(frames) {
        ValidateConfig(channels, mFrames);

        int alignedFrames = 0;
        int size = CalculateMemorySizeInternal(channels, frames, &alignedFrames);

        mData.reset(static_cast<float*>(AlignedAlloc(
                size, AudioBus::kChannelAlignment)));

        buildChannelData(channels, alignedFrames, mData.get());
    }

    AudioBus::~AudioBus() {

    }

    void AudioBus::buildChannelData(int channels, int alignedFrame, float* data) {
        DCHECK(IsAligned(data));
        DCHECK_EQ(mChannelData.size(), 0U);
        DCHECK_EQ(mChannelData.size(), 0U);
        // Initialize |mChannelData| with pointers into |data|.
        mChannelData.reserve(channels);
        for (int i = 0; i < channels; ++i)
            mChannelData.push_back(data + i * alignedFrame);
    }
}
