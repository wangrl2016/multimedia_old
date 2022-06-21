//
// Created by wang rl on 2022/6/13.
//

#include "base/time/Time.h"
#include "media/ffmpeg/FFmpegCommon.h"

namespace mm {
    static const AVRational kMicrosBase = {1, Time::kMicrosecondsPerSecond};

    int64_t ConvertFromTimeBase(const AVRational& timeBase,
                                int64_t timestamp) {
        int64_t microseconds = av_rescale_q(timestamp, timeBase, kMicrosBase);
        return microseconds;
    }

    int64_t ConvertToTimeBase(const AVRational& time_base,
                              const int64_t microseconds) {
        return av_rescale_q(microseconds, kMicrosBase, time_base);
    }
}