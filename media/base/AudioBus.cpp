//
// Created by wang rl on 2022/6/13.
//

#include <glog/logging.h>
#include "media/base/AudioBus.h"
#include "media/base/Limits.h"

namespace mm {
    static void ValidateConfig(int channels, int frames) {
        CHECK_GT(frames, 0);
        CHECK_GT(channels, 0);
        CHECK_LE(channels, static_cast<int>(kMaxChannels));
    }

    void AudioBus::CheckOverflow(int startFrame, int frames, int totalFrames) {
        CHECK_GE(startFrame, 0);
        CHECK_GE(frames, 0);
        CHECK_GT(totalFrames, 0);
        int sum = startFrame + frames;
        CHECK_LE(sum, totalFrames);
        CHECK_GE(sum, 0);
    }
}
