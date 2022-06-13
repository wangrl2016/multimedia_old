//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_FFMPEG_COMMON_H
#define MULTIMEDIA_FFMPEG_COMMON_H

#include <cstdint>

extern "C" {
#include <libavutil/mathematics.h>
#include <libavutil/rational.h>
};

namespace mm {
    // Converts an int64_t timestamp in |time_base| units to int64_t.
    // For example if |timestamp| equals 11025 and |timeBase| equals {1, 44100}
    // then the return value will be int64_t for 0.25 seconds since that
    // is how much time 11025/44100ths of a second represents.
    int64_t ConvertFromTimeBase(const AVRational& timeBase,
                                int64_t timestamp);
}

#endif //MULTIMEDIA_FFMPEG_COMMON_H
