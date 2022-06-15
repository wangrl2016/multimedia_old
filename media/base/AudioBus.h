//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_AUDIO_BUS_H
#define MULTIMEDIA_AUDIO_BUS_H

#include <vector>
#include "base/memory/AlignedMemory.h"

namespace mm {
    // Represents a sequence of audio frames containing frames() audio samples for
    // each of channels() channels. The data is stored as a set of contiguous
    // float arrays with one array per channel. The memory for the arrays is either
    // allocated and owned by the AudioBus or it is provided to one of the factory
    // methods. AudioBus guarantees that it allocates memory such that float array
    // for each channel is aligned by AudioBus::kChannelAlignment bytes, and it
    // requires the same for memory passed its Wrap...() factory methods.
    class AudioBus {
    public:
        // Guaranteed alignment of each channel's data; use 16-byte alignment for easy
        // SSE optimizations.
        enum {
            kChannelAlignment = 16  // 128 bits
        };

        // Creates a new AudioBus and allocates |channels| of length |frames|.
        static std::unique_ptr<AudioBus> Create(int channels, int frames);

        // Creates a new AudioBus from an existing channel vector. Does not transfer
        // ownership of |channelData| to AudioBus; i.e., |channelData| must outlive
        // the returned AudioBus. Each channel must be aligned by kChannelAlignment.
        static std::unique_ptr<AudioBus> WrapVector(
                int frames,
                const std::vector<float*>& channelData);

        // Creates a new AudioBus by wrapping an existing block of memory. Block must
        // be at least CalculateMemorySize() bytes in size. |data| must outlive the
        // returned AudioBus. |data| must be aligned by kChannelAlignment.
        static std::unique_ptr<AudioBus> WrapMemory(int channels,
                                                    int frames,
                                                    void* data);

        static std::unique_ptr<const AudioBus> WrapReadOnlyMemory(int channels,
                                                                  int frames,
                                                                  const void* data);

        // Based on the given number of channels and frames, calculates the minimum
        // required size in bytes of a contiguous block of memory to be passed to
        // AudioBus for storage of the audio data.
        static int CalculateMemorySize(int channels, int frames);

        // Overwrites the sample values stored in this AudioBus instance with values
        // from a given interleaved |sourceBuffer| with expected layout
        // [ch0, ch1, ..., chN, ch0, ch1, ...] and sample values in the format
        // corresponding to the given SourceSampleTypeTraits.
        // The sample values are converted to float values by means of the method
        // convertToFloat32() provided by the SourceSampleTypeTraits. For a list of
        // ready-to-use SampleTypeTraits, see file AudioSampleTypes.h.
        // If |numFramesToWrite| is less than frames(), the remaining frames are
        // zeroed out. If |numFramesToWrite| is more than frames(), write frames().
        template<class SourceSampleTypeTraits>
        void fromInterleaved(
                const typename SourceSampleTypeTraits::ValueType* sourceBuffer,
                int numFramesToWrite);

        // Similar to FromInterleaved...(), but overwrites the frames starting at a
        // given offset |writeOffsetInFrames| and does not zero out frames that are
        // not overwritten.
        template<class SourceSampleTypeTraits>
        void fromInterleavedPartial(
                const typename SourceSampleTypeTraits::ValueType* sourceBuffer,
                int writeOffsetInFrames,
                int numFramesToWrite);

        // Reads the sample values stored in this AudioBus instance and places them
        // into the given |destBuffer| in interleaved format using the sample format
        // specified by TargetSampleTypeTraits. For a list of ready-to-use
        // SampleTypeTraits, see file AudioSampleTypes.h. If |numFramesToRead| is
        // larger than frames(), write zero in last.
        template<class TargetSampleTypeTraits>
        void toInterleaved(
                int numFramesToRead,
                typename TargetSampleTypeTraits::ValueType* destBuffer) const;

        // Similar to toInterleaved(), but reads the frames starting at a given
        // offset |readOffsetInFrames|.
        template<class TargetSampleTypeTraits>
        void toInterleavedPartial(int readOffsetInFrames,
                                  int numFramesToRead,
                                  typename TargetSampleTypeTraits::ValueType destBuffer) const;

        // Helper method for copying channel data from one AudioBus to another.  Both
        // AudioBus object must have the same frames() and channels().
        void copyTo(AudioBus* dest) const;

        // Similar to above, but clips values to [-1, 1] during the copy process.
        void copyAndClipTo(AudioBus* dest) const;

        // Helper method to copy frames from one AudioBus to another. Both AudioBus
        // objects must have the same number of channels(). |sourceStartFrame| is
        // the starting offset. |destStartFrame| is the starting offset in |dest|.
        // |frameCount| is the number of frames to copy.
        void copyPartialFramesTo(int sourceStartFrame,
                                 int frameCount,
                                 int destStartFrame,
                                 AudioBus* dest) const;

        // Returns a raw pointer to the requested channel.  Pointer is guaranteed to
        // have a 16-byte alignment.  Warning: Do not rely on having sane (i.e. not
        // inf, nan, or between [-1.0, 1.0]) values in the channel data.
        float* channel(int channel) { return mChannelData[channel]; }

        const float* channel(int channel) const { return mChannelData[channel]; }

        // Returns the number of channels.
        int channels() const { return static_cast<int>(mChannelData.size()); }

        // Returns the number of frames.
        int frames() const { return mFrames; }

        // Helper method for zeroing out all channels of audio data.
        void zero();

        void zeroFrames(int frames);

        void zeroFramesPartial(int startFrame, int frames);

        // Checks if all frames are zero.
        bool areFramesZero() const;

        // Scale internal channel values by |volume| >= 0.  If an invalid value
        // is provided, no adjustment is done.
        void scale(float volume);

        // Swap channels identified by |a| and |b|.  The caller needs to make sure
        // the channels are valid.
        void swapChannels(int a, int b);

        AudioBus(const AudioBus&) = delete;

        AudioBus& operator=(const AudioBus&) = delete;

        virtual ~AudioBus();

    protected:
        AudioBus(int channels, int frames);

        AudioBus(int channels, int frames, float* data);

        AudioBus(int frames, const std::vector<float*>& channelData);

        explicit AudioBus(int channels);

    private:
        // Helper method for building |mChannelData| from a block of memory. |data|
        // must be at least CalculateMemorySize(...) bytes in size.
        void buildChannelData(int channels, int alignedFrame, float* data);

        static void CheckOverflow(int startFrame, int frames, int totalFrames);

        template<class SourceSampleTypeTraits>
        static void CopyConvertFromInterleavedSourceToAudioBus(
                const typename SourceSampleTypeTraits::ValueType* sourceBuffer,
                int writeOffsetInFrames,
                int numFramesToWrite,
                AudioBus* dest);

        template<class TargetSampleTypeTraits>
        static void CopyConvertFromAudioBusToInterleavedTarget(
                const AudioBus* source,
                int readOffsetInFrames,
                int numFramesToRead,
                typename TargetSampleTypeTraits::ValueType* destBuffer);

    private:
        // Contiguous block of channel memory.
        std::unique_ptr<float, AlignedFreeDeleter> mData;

        // One float pointer per channel pointing to a contiguous block of memory for
        // that channel. If the memory is owned by this instance, this will
        // point to the memory in |mData|. Otherwise, it may point to memory provided
        // by the client.
        std::vector<float*> mChannelData;

        int mFrames; // 数量
    };

    // template implementation
    template<class SourceSampleTypeTraits>
    void AudioBus::fromInterleaved(
            const typename SourceSampleTypeTraits::ValueType* sourceBuffer,
            int numFramesToWrite) {
        // Zero any remaining frames.
        zeroFramesPartial(numFramesToWrite,
                          mFrames - numFramesToWrite);
    }

    template<class SourceSampleTypeTraits>
    void AudioBus::fromInterleavedPartial(
            const typename SourceSampleTypeTraits::ValueType* sourceBuffer,
            int writeOffsetInFrames, int numFramesToWrite) {
        CheckOverflow(writeOffsetInFrames, numFramesToWrite, mFrames);
        CopyConvertFromInterleavedSourceToAudioBus<SourceSampleTypeTraits>(
                sourceBuffer, writeOffsetInFrames, numFramesToWrite, this);
    }

    // Delegates to toInterleavedPartial().
    template<class TargetSampleTypeTraits>
    void AudioBus::toInterleaved(
            int numFramesToRead,
            typename TargetSampleTypeTraits::ValueType* destBuffer) const {
        toInterleavedPartial<TargetSampleTypeTraits>(0,
                                                     numFramesToRead,
                                                     destBuffer);
    }

    template<class TargetSampleTypeTraits>
    void AudioBus::toInterleavedPartial(
            int readOffsetInFrames,
            int numFramesToRead,
            typename TargetSampleTypeTraits::ValueType dest) const {
        CheckOverflow(readOffsetInFrames, numFramesToRead, mFrames);
        CopyConvertFromAudioBusToInterleavedTarget<TargetSampleTypeTraits>(
                this, readOffsetInFrames, numFramesToRead, dest);
    }

    template<class SourceSampleTypeTraits>
    void AudioBus::CopyConvertFromInterleavedSourceToAudioBus(
            const typename SourceSampleTypeTraits::ValueType* sourceBuffer,
            int writeOffsetInFrames, int numFramesToWrite,
            AudioBus* dest) {
        const int channels = dest->channels();
        for (int ch = 0; ch < channels; ch++) {
            float* channelData = dest->channel(ch);
            for (int targetFrameIndex = writeOffsetInFrames,
                         readPosInSource = ch;
                 targetFrameIndex < writeOffsetInFrames + numFramesToWrite;
                 ++targetFrameIndex, readPosInSource += channels) {
                auto sourceValue = sourceBuffer[readPosInSource];
                channelData[targetFrameIndex] =
                        SourceSampleTypeTraits::ToFloat(sourceValue);
            }
        }
    }

    template<class TargetSampleTypeTraits>
    void AudioBus::CopyConvertFromAudioBusToInterleavedTarget(
            const AudioBus* source,
            int readOffsetInFrames,
            int numFramesToRead,
            typename TargetSampleTypeTraits::ValueType* destBuffer) {
        const int channels = source->channels();
        for (int ch = 0; ch < channels; ch++) {
            const float* channelData = source->channel(ch);
            for (int sourceFrameIndex = readOffsetInFrames, writePosInDest = ch;
                 sourceFrameIndex < readOffsetInFrames + numFramesToRead;
                 sourceFrameIndex++, writePosInDest += channels) {
                float sourceSampleValue = channelData[sourceFrameIndex];
                destBuffer[writePosInDest] =
                        TargetSampleTypeTraits::FromFloat(sourceSampleValue);
            }
        }
    }
}

#endif //MULTIMEDIA_AUDIO_BUS_H
