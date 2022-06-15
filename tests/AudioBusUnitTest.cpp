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

            for (int i = 0; i < bus->channels(); i++)
                verifyArrayIsFilledWithValue(bus->channel(i), bus->frames(), i);

            bus->zero();
            for (int i = 0; i < bus->channels(); i++)
                verifyArrayIsFilledWithValue(bus->channel(i), bus->frames(), 0);
        }

        // Verify copying to and from |bus1| and |bus2|.
        void copyTest(AudioBus* bus1, AudioBus* bus2) {
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

    // Verify an AudioBus created via wrapping a vector works as advertised.
    TEST_F(AudioBusTest, WrapVector) {
        mData.reserve(kChannels);
        for (int i = 0; i < kChannels; i++) {
            mData.push_back(static_cast<float*>(AlignedAlloc(
                    sizeof(*mData[i]) * kFrameCount, AudioBus::kChannelAlignment)));
        }

        std::unique_ptr<AudioBus> bus = AudioBus::WrapVector(kFrameCount, mData);
        verifyChannelAndFrameCount(bus.get());
        verifyReadWriteAndAlignment(bus.get());
    }

    // Verify an AudioBus created via wrapping a memory block works as advertised.
    TEST_F(AudioBusTest, WrapMemory) {
        int dataSize = AudioBus::CalculateMemorySize(kChannels, kFrameCount);
        std::unique_ptr<float, AlignedFreeDeleter> data(static_cast<float*>(
                                                                AlignedAlloc(dataSize, AudioBus::kChannelAlignment)));

        // Fill the memory with a test value we can check for after wrapping.
        static const float kTestValue = 3;
        std::fill(data.get(), data.get() + dataSize / sizeof(*data.get()), kTestValue);

        std::unique_ptr<AudioBus> bus = AudioBus::WrapMemory(kChannels, kFrameCount, data.get());
        // Verify the test value we filled prior to wrapping.
        for (int i = 0; i < bus->channels(); i++)
            verifyArrayIsFilledWithValue(bus->channel(i), bus->frames(), kTestValue);
        verifyChannelAndFrameCount(bus.get());
        verifyReadWriteAndAlignment(bus.get());

        // Verify the channel vectors lie within the provided memory block.
        EXPECT_GE(bus->channel(0), data.get());
        EXPECT_LT(bus->channel(bus->channels() - 1) + bus->frames(),
                  data.get() + dataSize / sizeof(*data.get()));
    }

    // Simulate a shared memory transfer and verify results.
    TEST_F(AudioBusTest, copyTo) {
        // Create one bus with AudioParameters and the other through direct values to
        // test for parity between the Create() functions.
        std::unique_ptr<AudioBus> bus1 = AudioBus::Create(kChannels, kFrameCount);
        std::unique_ptr<AudioBus> bus2 = AudioBus::Create(kChannels, kFrameCount);
        copyTest(bus1.get(), bus2.get());

        mData.reserve(kChannels);
        for (int i = 0; i < kChannels; i++) {
            mData.push_back(static_cast<float*>(AlignedAlloc(
                    sizeof(*mData[i]) * kFrameCount, AudioBus::kChannelAlignment)));
        }

        bus2 = AudioBus::WrapVector(kFrameCount, mData);
        copyTest(bus1.get(), bus2.get());

        // Try a copy to an AudioBus wrapping a memory block.
        std::unique_ptr<float, AlignedFreeDeleter> data(
                static_cast<float*>(AlignedAlloc(
                        AudioBus::CalculateMemorySize(kChannels, kFrameCount),
                        AudioBus::kChannelAlignment)));

        bus2 = AudioBus::WrapMemory(kChannels, kFrameCount, data.get());
        copyTest(bus1.get(), bus2.get());
    }

    // Verify zero() and zeroFrames(...) utility methods work as advertised.
    TEST_F(AudioBusTest, zero) {
        std::unique_ptr<AudioBus> bus = AudioBus::Create(kChannels, kFrameCount);

        // Fill the bus with dummy data.
        for (int i = 0; i < bus->channels(); i++)
            std::fill(bus->channel(i), bus->channel(i) + bus->frames(), i + 1);
        EXPECT_FALSE(bus->areFramesZero());

        // Zero first half the frames of each channel.
        bus->zeroFrames(kFrameCount / 2);
        for (int i = 0; i < bus->channels(); i++) {
            verifyArrayIsFilledWithValue(bus->channel(i), kFrameCount / 2, 0);
            verifyArrayIsFilledWithValue(bus->channel(i) + kFrameCount / 2,
                                         kFrameCount - kFrameCount / 2, i + 1);
        }
        EXPECT_FALSE(bus->areFramesZero());

        // Fill the bus with dummy data.
        for (int i = 0; i < bus->channels(); i++)
            std::fill(bus->channel(i), bus->channel(i) + bus->frames(), i + 1);

        // Zero the last half of the frames.
        bus->zeroFramesPartial(kFrameCount / 2, kFrameCount - kFrameCount / 2);
        for (int i = 0; i < bus->channels(); i++) {
            verifyArrayIsFilledWithValue(bus->channel(i) + kFrameCount / 2,
                                         kFrameCount - kFrameCount / 2, 0);
            verifyArrayIsFilledWithValue(bus->channel(i), kFrameCount / 2, i + 1);
        }
        EXPECT_FALSE(bus->areFramesZero());

        // Fill the bus with dummy data.
        for (int i = 0; i < bus->channels(); i++)
            std::fill(bus->channel(i), bus->channel(i) + bus->frames(), i + 1);

        // Zero all the frames of each channel.
        bus->zero();
        for (int i = 0; i < bus->channels(); i++) {
            verifyArrayIsFilledWithValue(bus->channel(i), bus->frames(), 0);
        }
        EXPECT_TRUE(bus->areFramesZero());
    }

    // Each test vector represents two channels of data in the following arbitrary
    // layout: <min, zero, max, min, max / 2, min / 2, zero, max, zero, zero>.
    static const int kTestVectorSize = 10;
    static const uint8_t kTestVectorUint8[kTestVectorSize] = {
            0, -INT8_MIN, UINT8_MAX,
            0, INT8_MAX / 2 + 128, INT8_MIN / 2 + 128,
            -INT8_MIN, UINT8_MAX, -INT8_MIN,
            -INT8_MIN
    };

    static const int16_t kTestVectorInt16[kTestVectorSize] = {
            INT16_MIN, 0, INT16_MAX, INT16_MIN, INT16_MAX / 2,
            INT16_MIN / 2, 0, INT16_MAX, 0, 0
    };

    static const int32_t kTestVectorInt32[kTestVectorSize] = {
            INT32_MIN, 0, INT32_MAX, INT32_MIN, INT32_MAX / 2,
            INT32_MIN / 2, 0, INT32_MAX, 0, 0};

    static const float kTestVectorFloat32[kTestVectorSize] = {
            -1.0f, 0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f};

    // This is based on kTestVectorFloat32, but has some values outside of
    // sanity.
    static const float kTestVectorFloat32Invalid[kTestVectorSize] = {
            -5.0f,
            0.0f,
            5.0f,
            -1.0f,
            0.5f,
            -0.5f,
            0.0f,
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::signaling_NaN(),
            std::numeric_limits<float>::quiet_NaN()};

    static const float kTestVectorFloat32Sanitized[kTestVectorSize] = {
            -1.0f, 0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.0f, 1.0f, -1.0f, -1.0f};

    // Expected results.
    static const int kTestVectorFrameCount = kTestVectorSize / 2;
    static const float kTestVectorResult[][kTestVectorFrameCount] = {
            {-1.0f, 1.0f,  0.5f,  0.0f, 0.0f},
            {0.0f,  -1.0f, -0.5f, 1.0f, 0.0f}};
    static const int kTestVectorChannelCount = std::size(kTestVectorResult);

    // Verify fromInterleaved() de-interleaves audio in supported formats correctly.
    TEST_F(AudioBusTest, fromInterleaved) {
        std::unique_ptr<AudioBus> bus =
                AudioBus::Create(kTestVectorChannelCount, kTestVectorFrameCount);
        std::unique_ptr<AudioBus> expected =
                AudioBus::Create(kTestVectorChannelCount, kTestVectorFrameCount);

        for (int ch = 0; ch < kTestVectorChannelCount; ch++) {
            memcpy(expected->channel(ch), kTestVectorResult[ch],
                   kTestVectorFrameCount * sizeof(*expected->channel(ch)));
        }

        bus->zero();
        bus->fromInterleaved<>()
    }
}
