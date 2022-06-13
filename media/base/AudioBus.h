//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_AUDIO_BUS_H
#define MULTIMEDIA_AUDIO_BUS_H

#include <vector>
#include "base/memory/AlignedMemory.h"

namespace mm {
    class AudioBus {
    public:
        // Guaranteed alignment of each channel's data; use 16-byte alignment for easy
        // SSE optimizations.
        enum {
            kChannelAlignment = 16
        };

        static std::unique_ptr<AudioBus> Create(int channels, int frames);
        
        void setChannelData(int channel, float* data);

        void setFrames(int frames);

        // Helper method for copying channel data from one AudioBus to another.  Both
        // AudioBus object must have the same frames() and channels().
        void copyTo(AudioBus* dest) const;

        // Returns a raw pointer to the requested channel.  Pointer is guaranteed to
        // have a 16-byte alignment.  Warning: Do not rely on having sane (i.e. not
        // inf, nan, or between [-1.0, 1.0]) values in the channel data.
        float* channel(int channel) { return mChannelData[channel]; }

        const float* channel(int channel) const { return mChannelData[channel]; }

        int channels() const { return static_cast<int>(mChannelData.size()); }

        // Returns the number of frames.
        int frames() const { return mFrames; }

        // Helper method for zeroing out all channels of audio data.
        void zero();

        void zeroFrames(int frames);

        void zeroFramesPartial(int start_frame, int frames);

        // Checks if all frames are zero.
        bool areFramesZero() const;

        // Scale internal channel values by |volume| >= 0.  If an invalid value
        // is provided, no adjustment is done.
        void scale(float volume);

        // Swap channels identified by |a| and |b|.  The caller needs to make sure
        // the channels are valid.
        void swapChannels(int a, int b);

        virtual ~AudioBus();

    protected:
        AudioBus(int channels, int frames);

        AudioBus(int channels, int frames, float* data);

        AudioBus(int frames, const std::vector<float*>& channelData);

        explicit AudioBus(int channels);

    private:
        static void CheckOverflow(int startFrame, int frames, int totalFrames);

    private:
        std::unique_ptr<float, AlignedFreeDeleter> mData;

        // One float pointer per channel pointing to a contiguous block of memory for
        // that channel. If the memory is owned by this instance, this will
        // point to the memory in |mData|. Otherwise, it may point to memory provided
        // by the client.
        std::vector<float*> mChannelData;
        int mFrames;
    };
}

#endif //MULTIMEDIA_AUDIO_BUS_H
