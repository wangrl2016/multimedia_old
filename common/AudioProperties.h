//
// Created by WangRuiLing on 2022/6/13.
//

#ifndef MULTIMEDIA_AUDIO_PROPERTIES_H
#define MULTIMEDIA_AUDIO_PROPERTIES_H

extern "C" {
#include <libavformat/avformat.h>
};

namespace mm {
    struct AudioProperties {
        int channelCount;
        int sampleRate;
        AVSampleFormat sampleFormat;

        // 打印音频属性
        void dump() const;
    };
}

#endif //MULTIMEDIA_AUDIO_PROPERTIES_H
