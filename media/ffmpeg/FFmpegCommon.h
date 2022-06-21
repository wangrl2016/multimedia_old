//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_FFMPEG_COMMON_H
#define MULTIMEDIA_FFMPEG_COMMON_H

#include <cstdint>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/rational.h>
}

#include "media/ffmpeg/ffmpeg_deleters.h"

namespace mm {
    // Converts an int64_t timestamp in |time_base| units to int64_t.
    // For example if |timestamp| equals 11025 and |timeBase| equals {1, 44100}
    // then the return value will be int64_t for 0.25 seconds since that
    // is how much time 11025/44100ths of a second represents.
    int64_t ConvertFromTimeBase(const AVRational& timeBase,
                                int64_t timestamp);

    // Converts an int64_t into an int64_t timestamp in |time_base| units.
    // For example if |timestamp| is 0.5 seconds and |time_base| is {1, 44100}, then
    // the return value will be 22050 since that is hwo many 1/44100ths of a second
    // represent 0.5 seconds.
    int64_t ConvertToTimeBase(const AVRational& time_base,
                              const int64_t microseconds);

    std::unique_ptr<AVCodecContext, ScopedPtrAVFreeContext>
            AVStreamToAVCodecContext(const AVStream* stream) {
        std::unique_ptr<AVCodecContext, ScopedPtrAVFreeContext> codec_context(
                avcodec_alloc_context3(nullptr));
        if (avcodec_parameters_to_context(codec_context.get(), stream->codecpar) <
            0) {
            return nullptr;
        }

        return codec_context;
    }

}

#endif //MULTIMEDIA_FFMPEG_COMMON_H
