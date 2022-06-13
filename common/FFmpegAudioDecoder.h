//
// Created by WangRuiLing on 2022/6/13.
//

#ifndef MULTIMEDIA_FFMPEG_AUDIO_DECODER_H
#define MULTIMEDIA_FFMPEG_AUDIO_DECODER_H

#include <string>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
#include "common/AudioProperties.h"

namespace mm {
    class FFmpegAudioDecoder {
    public:
        bool open(const std::string& filePath);

        // These methods can be called once open() has been called.
        int srcChannelCount() const { return mSrcChannelCount; }

        int srcSampleRate() const { return mSrcSampleRate; }

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

        void close();
    private:
        AVFormatContext* mFormatCtx = nullptr;
        AVCodecContext* mCodecCtx = nullptr;
        SwrContext* mSwrCtx = nullptr;
        const AVCodec* mCodec = nullptr;

        int mStreamIndex = -1;

        int mSrcChannelCount = -1;
        int mSrcSampleRate = -1;

        AudioProperties mSrcAudioProperties;
    };
}

#endif //MULTIMEDIA_FFMPEG_AUDIO_DECODER_H
