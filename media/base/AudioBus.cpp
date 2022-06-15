//
// Created by wang rl on 2022/6/13.
//

#include <glog/logging.h>
#include "media/base/AudioBus.h"
#include "media/base/Limits.h"

namespace mm {
    static bool IsAligned(void* ptr) {
        return (reinterpret_cast<uintptr_t>(ptr) &
                (AudioBus::kChannelAlignment - 1)) == 0U;
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
                int(((frames * sizeof(float) + AudioBus::kChannelAlignment - 1) &
                     ~(AudioBus::kChannelAlignment - 1))) / sizeof(float);

        if (outAlignedFrames)
            *outAlignedFrames = alignedFrames;

        return sizeof(float) * channels * alignedFrames;
    }

    std::unique_ptr<AudioBus> AudioBus::Create(int channels, int frames) {
        return std::unique_ptr<AudioBus>(new AudioBus(channels, frames));
    }

    std::unique_ptr<AudioBus> AudioBus::WrapVector(
            int frames,
            const std::vector<float*>& channelData) {
        return std::unique_ptr<AudioBus>(new AudioBus(frames, channelData));
    }

    std::unique_ptr<AudioBus> AudioBus::WrapMemory(int channels,
                                                   int frames,
                                                   void* data) {
        // |data| must be aligned by AudioBus::kChannelAlignment.
        CHECK(IsAligned(data));
        return std::unique_ptr<AudioBus>(new AudioBus(
                channels, frames, static_cast<float*>(data)));
    }

    std::unique_ptr<const AudioBus> AudioBus::WrapReadOnlyMemory(
            int channels, int frames, const void* data) {
        // Note: const_cast is generally dangerous but is used in this case since
        // AudioBus accommodates both read-only and read/write use cases. A const
        // AudioBus object is returned to ensure no one accidentally writes to the
        // read-only data.
        return WrapMemory(channels, frames, const_cast<void*>(data));
    }

    int AudioBus::CalculateMemorySize(int channels, int frames) {
        return CalculateMemorySizeInternal(channels, frames, nullptr);
    }

    static void ValidateConfig(int channels, int frames) {
        CHECK_GT(frames, 0);
        CHECK_GT(channels, 0);
        CHECK_LE(channels, static_cast<int>(kMaxChannels));
    }

    void AudioBus::copyTo(AudioBus* dest) const {
        copyPartialFramesTo(0, frames(), 0, dest);
    }

    void AudioBus::copyAndClipTo(AudioBus* dest) const {
        CHECK_EQ(channels(), dest->channels());
        CHECK_LE(frames(), dest->frames());
        for (int i = 0; i < channels(); i++) {
            float* destPtr = dest->channel(i);
            const float* sourcePtr = channel(i);
            for (int j = 0; j < frames(); j++)
                destPtr[j] = sourcePtr[j] < -1.0f ? -1.0f : sourcePtr[j] > 1.0f ? 1.0f : sourcePtr[j];
        }
    }

    void AudioBus::copyPartialFramesTo(int sourceStartFrame,
                                       int frameCount,
                                       int destStartFrame,
                                       AudioBus* dest) const {
        CHECK_EQ(channels(), dest->channels());
        CHECK_LE(sourceStartFrame + frameCount, frames());
        CHECK_LE(destStartFrame + frameCount, dest->frames());

        // Since we don't know if the other AudioBus is wrapped or not (and we don't
        // want to care), just copy using the public channel() accessors.
        for (int i = 0; i < channels(); i++) {
            memcpy(dest->channel(i) + destStartFrame,
                   channel(i) + sourceStartFrame,
                   sizeof(*channel(i)) * frameCount);
        }
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

    void AudioBus::swapChannels(int a, int b) {
        DCHECK(a < channels() && a >= 0);
        DCHECK(b < channels() && b >= 0);
        DCHECK_NE(a, b);
        std::swap(mChannelData[a], mChannelData[b]);
    }

    AudioBus::~AudioBus() {

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

    AudioBus::AudioBus(int channels, int frames, float* data)
            : mFrames(frames) {
        // Since |data| may have come from an external source, ensure it's valid.
        CHECK(data);
        ValidateConfig(channels, frames);

        int alignedFrames = 0;
        CalculateMemorySizeInternal(channels, frames, &alignedFrames);

        buildChannelData(channels, alignedFrames, data);
    }

    AudioBus::AudioBus(int frames, const std::vector<float*>& channelData)
            : mChannelData(channelData), mFrames(frames) {
        ValidateConfig(int(mChannelData.size()), mFrames);

        // Sanity check wrapped vector for alignment and channel count.
        for (size_t i = 0; i < mChannelData.size(); i++) {
            DCHECK(IsAligned(mChannelData[i]));
        }
    }

    AudioBus::AudioBus(int channels)
            : mChannelData(channels), mFrames(0) {
        CHECK_GT(channels, 0);
        for (size_t i = 0; i < mChannelData.size(); i++)
            mChannelData[i] = nullptr;
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

    void AudioBus::CheckOverflow(int startFrame, int frames, int totalFrames) {
        CHECK_GE(startFrame, 0);
        CHECK_GE(frames, 0);
        CHECK_GT(totalFrames, 0);
        int sum = startFrame + frames;
        CHECK_LE(sum, totalFrames);
        CHECK_GE(sum, 0);
    }
}
