//
// Created by WangRuiLing on 2022/6/13.
//

#include <glog/logging.h>
#include "common/AudioProperties.h"

namespace mm {
    void AudioProperties::dump() const {
        LOG(INFO) << "channelCount " << channelCount
                << ", sampleRate " << sampleRate
                << ", sampleFormat " << av_get_sample_fmt_name(sampleFormat);
    }
}