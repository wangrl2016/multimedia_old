//
// Created by WangRuiLing on 2022/6/14.
//

#include <memory>
#include <gtest/gtest.h>
extern "C" {
#include <libavutil/channel_layout.h>
}
#include "media/base/AudioBus.h"

namespace mm {
    static const int kChannels = 6;
    static const int64_t kChannelLayout = AV_CH_LAYOUT_5POINT1;
    // Use a buffer size which is intentionally not a multiple of kChannelAlignment.
    static const int kFrameCount = AudioBus::kChannelAlignment * 32 - 1;
    static const int kSampleRate = 48000;

    class AudioBusTest : public testing::Test {
    public:
        AudioBusTest() = default;

        AudioBusTest(const AudioBusTest&) = delete;

        AudioBusTest& operator=(const AudioBusTest&) = delete;

        ~AudioBusTest() override {
            for (size_t i = 0; i < mData.size(); i++) {
                AlignedFree(mData[i]);
            }
        }

        void verifyChannelAndFrameCount(AudioBus* bus) {
            EXPECT_EQ(kChannels, bus->channels());
            EXPECT_EQ(kFrameCount, bus->frames());
        }
    protected:
        std::vector<float*> mData;
    };

    // Verify basic Create(...) method works as advertised.
    TEST_F(AudioBusTest, Create) {
        std::unique_ptr<AudioBus> bus = AudioBus::Create(kChannels, kFrameCount);
        verifyChannelAndFrameCount(bus.get());

    }
}
