//
// Created by WangRuiLing on 2022/6/14.
//

#ifndef MULTIMEDIA_CONSTANTS_H
#define MULTIMEDIA_CONSTANTS_H

extern "C" {
#include <libavformat/avformat.h>
};

namespace mm {
    constexpr int kDefaultChannelCount = 2;
    constexpr int kDefaultSampleRate = 44100;
    constexpr AVSampleFormat kDefaultSampleFormat = AV_SAMPLE_FMT_FLTP;
}

#endif //MULTIMEDIA_CONSTANTS_H
