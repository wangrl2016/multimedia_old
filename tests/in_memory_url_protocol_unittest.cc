//
// Created by WangRuiLing on 2022/6/21.
//

#include <gtest/gtest.h>
#include "media/filters/in_memory_url_protocol.h"

namespace mm {
    static const uint8_t kData[] = {0x01, 0x02, 0x03, 0x04};

    TEST(InMemoryUrlProtocolTest, ReadFromLargeBuffer) {
        InMemoryUrlProtocol protocol(kData, std::numeric_limits<int64_t>::max(),
                                     false);

        uint8_t out[sizeof(kData)];
        EXPECT_EQ(4, protocol.Read(sizeof(out), out));
        EXPECT_EQ(0, memcmp(out, kData, sizeof(out)));
    }

    TEST(InMemoryUrlProtocolTest, ReadWithNegativeSize) {
        InMemoryUrlProtocol protocol(kData, sizeof(kData), false);

        uint8_t out[sizeof(kData)];
        EXPECT_EQ(AVERROR(EIO), protocol.Read(-2, out));
    }

    TEST(InMemoryUrlProtocolTest, ReadWithZeroSize) {
        InMemoryUrlProtocol protocol(kData, sizeof(kData), false);

        uint8_t out;
        EXPECT_EQ(0, protocol.Read(0, &out));
    }

    TEST(InMemoryUrlProtocolTest, SetPosition) {
        InMemoryUrlProtocol protocol(kData, sizeof(kData), false);

        EXPECT_FALSE(protocol.SetPosition(-1));
        EXPECT_FALSE(protocol.SetPosition(sizeof(kData) + 1));

        uint8_t out;
        EXPECT_TRUE(protocol.SetPosition(sizeof(kData)));
        EXPECT_EQ(AVERROR_EOF, protocol.Read(1, &out));

        int i = sizeof(kData) / 2;
        EXPECT_TRUE(protocol.SetPosition(i));
        EXPECT_EQ(1, protocol.Read(1, &out));
        EXPECT_EQ(kData[i], out);
    }
}