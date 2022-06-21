//
// Created by WangRuiLing on 2022/6/21.
//

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include "media/filters/audio_file_reader.h"
#include "media/filters/in_memory_url_protocol.h"

namespace mm {
    class AudioFileReaderTest : public testing::Test {
    public:
        AudioFileReaderTest() : packet_verification_disabled_(false) {}

        AudioFileReaderTest(const AudioFileReaderTest&) = delete;

        AudioFileReaderTest& operator=(const AudioFileReaderTest&) = delete;

        ~AudioFileReaderTest() override = default;

        void Initialize(const char* filename) {
            // 将文件读入到内存中
            size_ = std::filesystem::file_size(filename);
            auto* data = (uint8_t*)calloc(size_, sizeof(uint8_t));
            std::ifstream ifs;
            ifs.open(filename, std::ios::binary);
            ifs.read((char*)data, size_);
            ifs.close();
            data_ = std::unique_ptr<uint8_t[]>(data);
            protocol_ = std::make_unique<InMemoryUrlProtocol>(
                    data_.get(), size_, false);
            reader_ = std::make_unique<AudioFileReader>(protocol_.get());
        }

        // Reads and the entire file provide to Initialize().
        void ReadAndVerify(const char* expected_audio_hash, int expected_frames) {
            std::vector<std::unique_ptr<AudioBus>> decoded_audio_packets;
            int actual_frames = reader_->Read(&decoded_audio_packets);
            std::unique_ptr<AudioBus> decoded_audio_data =
                    AudioBus::Create(reader_->channels(), actual_frames);
            int dest_start_frame = 0;
            for (size_t k = 0; k < decoded_audio_packets.size(); ++k) {
                const AudioBus* packet = decoded_audio_packets[k].get();
                int frame_count = packet->frames();
                packet->copyPartialFramesTo(0, frame_count, dest_start_frame,
                                            decoded_audio_data.get());
                dest_start_frame += frame_count;
            }
            ASSERT_LE(actual_frames, decoded_audio_data->frames());
            ASSERT_EQ(expected_frames, actual_frames);
        }

        void RunTestPartialDecode(const char* filename) {
            Initialize(filename);
            EXPECT_TRUE(reader_->Open());
        }

    protected:
        std::unique_ptr<uint8_t[]> data_;
        // Size of the encoded data.
        size_t size_;
        std::unique_ptr<InMemoryUrlProtocol> protocol_;
        std::unique_ptr<AudioFileReader> reader_;
        bool packet_verification_disabled_;
    };

    TEST_F(AudioFileReaderTest, ReadPartialMP3) {
        // RunTestPartialDecode("res/fltp_1_44100.mp3");
    }
}