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

        virtual ~AudioBus();
    protected:
        AudioBus(int channels, int frames);

        AudioBus(int channels, int frames, float* data);

        AudioBus(int frames, const std::vector<float*>& channelData);

        explicit AudioBus(int channels);
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
