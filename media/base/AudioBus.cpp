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
}
