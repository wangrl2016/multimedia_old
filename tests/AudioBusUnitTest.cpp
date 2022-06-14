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

        void verifyArrayIsFilledWithValue(const float data[], int size, float value) {
            for (int i = 0; i < size; ++i)
                ASSERT_FLOAT_EQ(value, data[i]) << "i = " << i;
        }

        void verifyAreEqualWithEpsilon(const AudioBus* result,
                                       const AudioBus* expected,
                                       float epsilon) {
            ASSERT_EQ(expected->channels(), result->channels());
            ASSERT_EQ(expected->frames(), result->frames());

            for (int ch = 0; ch < result->channels(); ch++) {
                for (int i = 0; i < result->frames(); i++) {
                    if (epsilon == 0) {
                        ASSERT_FLOAT_EQ(expected->channel(ch)[i], result->channel(ch)[i]);
                    } else {
                        ASSERT_NEAR(expected->channel(ch)[i], result->channel(ch)[i],
                                    epsilon);
                    }
                }
            }
        }

        // Verify values for each channel in |result| against |expected|.
        void verifyAreEqual(const AudioBus* result, const AudioBus* expected) {
            verifyAreEqualWithEpsilon(result, expected, 0);
        }

        // Read and write to the full extent of the allocated channel data. Also test
        // the Zero() method and verify it does as advertised. Also test data if data
        // is 16-bytes aligned as advertised (see kChannelAlignment in AudioBus.h).
        void verifyReadWriteAndAlignment(AudioBus* bus) {
            for (int i = 0; i < bus->channels(); i++) {
                // Verify that the address returned by channel(i) is a multiple of
                // AudioBus::kChannelAlignment.
                ASSERT_EQ(0U, reinterpret_cast<uintptr_t>(
                        bus->channel(i)) & (AudioBus::kChannelAlignment - 1));

                // Write into the channel buffer.
                std::fill(bus->channel(i), bus->channel(i) + bus->frames(), i);
            }

            for (int i = 0; i< bus->channels(); i++)
                verifyArrayIsFilledWithValue(bus->channel(i), bus->frames(), i);

            bus->zero();
            for (int i = 0; i < bus->channels(); i++)
                verifyArrayIsFilledWithValue(bus->channel(i), bus->frames(), 0);
        }

        // Verify copying to and from |bus1| and |bus2|.
        void CopyTest(AudioBus* bus1, AudioBus* bus2) {
            // Fill |bus1| with dummy data.
            for (int i = 0; i < bus1->channels(); i++)
                std::fill(bus1->channel(i), bus1->channel(i) + bus1->frames(), i);

            // Verify copy from |bus1| to |bus2|.
            bus2->zero();
            bus1->copyTo(bus2);
            verifyAreEqual(bus1, bus2);

            // Verify copy from |bus2| to |bus1|.
            bus1->zero();
            bus2->copyTo(bus1);
            verifyAreEqual(bus2, bus1);
        }
    protected:
        std::vector<float*> mData;
    };

    // Verify basic Create(...) method works as advertised.
    TEST_F(AudioBusTest, Create) {
        std::unique_ptr<AudioBus> bus = AudioBus::Create(kChannels, kFrameCount);
        verifyChannelAndFrameCount(bus.get());
        verifyReadWriteAndAlignment(bus.get());
    }
}
