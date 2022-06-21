//
// Created by WangRuiLing on 2022/6/21.
//

#include <gtest/gtest.h>
#include "media/filters/audio_file_reader.h"
#include "media/filters/in_memory_url_protocol.h"

namespace mm {
    class AudioFileReaderTest : public testing::Test {
    protected:
        std::unique_ptr<uint8_t[]> data_;
        // Size of the encoded data.
        size_t size_;
        std::unique_ptr<InMemoryUrlProtocol> protocol_;
        std::unique_ptr<AudioFileReader> reader_;
        bool packet_verification_disabled_;
    };
}