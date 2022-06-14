//
// Created by WangRuiLing on 2022/6/13.
//

#ifndef MULTIMEDIA_FFMPEG_AUDIO_DECODER_H
#define MULTIMEDIA_FFMPEG_AUDIO_DECODER_H

#include <limits>
#include <string>
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include "common/AudioProperties.h"
#include "media/base/AudioBus.h"

namespace mm {
    class FFmpegAudioDecoder {
    public:
        FFmpegAudioDecoder();

        ~FFmpegAudioDecoder();

        bool open(const std::string& filePath);

        // These methods can be called once open() has been called.
        int srcChannelCount() const { return mSrcAudioProperties.channelCount; }

        int srcSampleRate() const { return mSrcAudioProperties.sampleRate; }

        // Return true if (an estimated) duration of the audio data is
        // known. Must be called after open().
        bool hasKnownDuration() const;

        // Please note that getDuration() and getNumberOfFrames() attempt to be
        // accurate, but are only estimates. For some encoded formats, the actual
        // duration of the file can only be determined once all the file data has been
        // read. The read() method returns the actual number of sample-frames it
        // has read.
        int64_t getDuration() const;

        int getNumberOfFrames() const;

        AudioProperties getSrcAudioProperties() const { return mSrcAudioProperties; }

        bool setDestAudioProperties(AudioProperties& audioProperties);

        // After a call to open(), attempts to decode the data,
        // updating |decodedAudioPackets| with each decoded packet in order.
        // The caller must convert these packets into one complete set of
        // decoded audio data. The audio data will be decoded as
        // floating-point linear PCM with a nominal range of -1.0 -> +1.0.
        // Returns the number of sample-frames actually read which will
        // always be the total size of all the frames in
        // |decodedAudioPackets|.
        int read(std::vector<std::unique_ptr<AudioBus>>* decodedAudioPackets);

        void close();

    private:
        AVFormatContext* mFormatCtx = nullptr;
        AVCodecContext* mCodecCtx = nullptr;
        SwrContext* mSwrCtx = nullptr;
        const AVCodec* mCodec = nullptr;

        int mStreamIndex = -1;

        AudioProperties mSrcAudioProperties{};
        AudioProperties mDestAudioProperties{};
    };
}

#endif //MULTIMEDIA_FFMPEG_AUDIO_DECODER_H
