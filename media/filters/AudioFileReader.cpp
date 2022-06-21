//
// Created by WangRuiLing on 2022/6/20.
//

#include "base/time/Time.h"
#include "media/base/AudioSampleTypes.h"
#include "media/ffmpeg/FFmpegCommon.h"
#include "media/filters/AudioFileReader.h"

namespace mm {
    // AAC(M4A) decoding specific constants.
    static const int kAACPrimingFrameCount = 2112;
    static const int kAACRemainderFrameCount = 519;

    AudioFileReader::AudioFileReader(FFmpegURLProtocol* protocol)
            : stream_index_(0),
              protocol_(protocol),
              audio_codec_(AV_CODEC_ID_FIRST_UNKNOWN),
              channels_(0),
              sample_rate_(0),
              av_sample_format_(0) {}

    AudioFileReader::~AudioFileReader() {
        Close();
    }

    bool AudioFileReader::Open() {
        return OpenDemuxer() && OpenDecoder();
    }

    void AudioFileReader::Close() {
        codec_context_.reset();
        glue_.reset();
    }

    int AudioFileReader::Read(
            std::vector<std::unique_ptr<AudioBus>>* decoded_audio_packets,
            int packets_to_read) {
        DCHECK(glue_ && codec_context_)
                            << "AudioFileReader::Read() : reader is not opened!";
        int total_frames = 0;
        AVPacket packet;
        std::unique_ptr<AVFrame, ScopedPtrAVFreeFrame> frame(av_frame_alloc());
        int packets_read = 0;
        DecodeStatus status = DecodeStatus::kOkay;
        while (packets_read++ < packets_to_read && ReadPacket(&packet)) {
            // 进行解码操作
            bool sent_packet = false, frames_remaining = true, decoder_error = false;
            while (!sent_packet || frames_remaining) {
                if (!sent_packet) {
                    const int result = avcodec_send_packet(codec_context_.get(), &packet);
                    if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
                        DLOG(ERROR) << "Failed to send packet for decoding: " << result;
                        status = DecodeStatus::kSendPacketFailed;
                        // 退出里层的while循环
                        break;
                    }
                    sent_packet = result != AVERROR(EAGAIN);
                }

                // See if any frames are available. If we receive an EOF or EAGAIN, there
                // should be nothing left to do this pass since we've already provided the
                // only input packet that we have.
                const int result = avcodec_receive_frame(codec_context_.get(), frame.get());
                if (result == AVERROR_EOF || result == AVERROR(EAGAIN)) {
                    frames_remaining = false;
                    if (result == AVERROR(EAGAIN)) {
                        CHECK(sent_packet) << "avcodec_receive_frame() and "
                                              "avcodec_send_packet() both returned EAGAIN, "
                                              "which is an API violation.";
                    }
                    continue;
                } else if (result < 0) {
                    DLOG(ERROR) << "Failed to decode frame: " << result;
                    decoder_error = true;
                    status = DecodeStatus::kDecodeFrameFailed;
                    continue;
                }

                const bool frame_processing_success =
                        OnNewFrame(&total_frames, decoded_audio_packets, frame.get());
                av_frame_unref(frame.get());
                if (!frame_processing_success) {
                    status = DecodeStatus::kFrameProcessingFailed;
                } else {
                    status = DecodeStatus::kOkay;
                }
            }
            av_packet_unref(&packet);
            if (status != DecodeStatus::kOkay)
                break;
        }
        return total_frames;
    }

    bool AudioFileReader::HasKnownDuration() const {
        return glue_->format_context()->duration != AV_NOPTS_VALUE;
    }

    int64_t AudioFileReader::GetDuration() const {
        const AVRational av_time_base = {1, AV_TIME_BASE};
        DCHECK_NE(glue_->format_context()->duration, AV_NOPTS_VALUE);
        int64_t estimated_duration_us = glue_->format_context()->duration;

        if (audio_codec_ == AV_CODEC_ID_AAC) {
            // For certain AAC-encoded files, FFmpeg estimated frame count might not
            // be sufficient to capture the entire audio content that we want. This is
            // especially noticeable for short files (< 10ms) resulting in silence
            // throughout the decoded buffer. Thus, we add the priming frames and the
            // remainder frames to the estimation.
            estimated_duration_us += ceil(
                    1000000.0 *
                    static_cast<double>(kAACPrimingFrameCount + kAACRemainderFrameCount) /
                    sample_rate());
        } else {
            // Add one microsecond to avoid rounding-down errors which can occur when
            // |duration| has been calculated from an exact number of sample-frames.
            // One microsecond is much less than the time of a single sample-frame
            // at any real-world sample-rate.
            estimated_duration_us += 1;
        }

        return ConvertFromTimeBase(av_time_base, estimated_duration_us);
    }

    int AudioFileReader::GetNumberOfFrames() const {
        return std::ceil((double(GetDuration()) / Time::kMicrosecondsPerSecond) * sample_rate());
    }

    bool AudioFileReader::OpenDemuxerForTesting() {
        return OpenDemuxer();
    }

    bool AudioFileReader::ReadPacketForTesting(AVPacket* output_packet) {
        return ReadPacket(output_packet);
    }

    bool AudioFileReader::SeekForTesting(int64_t seek_time) {
        // Use the AVStream's time_base, since |codec_context_| does not have
        // time_base populated until after OpenDecoder().
        return av_seek_frame(
                glue_->format_context(), stream_index_,
                ConvertToTimeBase(GetAVStreamForTesting()->time_base, seek_time),
                AVSEEK_FLAG_BACKWARD) >= 0;
    }

    const AVStream* AudioFileReader::GetAVStreamForTesting() const {
        return glue_->format_context()->streams[stream_index_];
    }

    bool AudioFileReader::OpenDemuxer() {
        glue_ = std::make_unique<FFmpegGlue>(protocol_);
        AVFormatContext* format_context = glue_->format_context();

        // Open FFmpeg AVFormatContext.
        if (!glue_->OpenContext()) {
            DLOG(WARNING) << "AudioFileReader::Open() : error in avformat_open_input()";
            return false;
        }

        const int result = avformat_find_stream_info(format_context, NULL);
        if (result < 0) {
            DLOG(WARNING)
                            << "AudioFileReader::Open() : error in avformat_find_stream_info()";
            return false;
        }

        // Calling avformat_find_stream_info can uncover new streams. We wait till now
        // to find the first audio stream, if any.
        codec_context_.reset();
        bool found_stream = false;
        for (size_t i = 0; i < format_context->nb_streams; ++i) {
            if (format_context->streams[i]->codecpar->codec_type ==
                AVMEDIA_TYPE_AUDIO) {
                stream_index_ = i;
                found_stream = true;
                break;
            }
        }

        if (!found_stream)
            return false;

        // Get the codec context.
        codec_context_ =
                AVStreamToAVCodecContext(format_context->streams[stream_index_]);
        if (!codec_context_)
            return false;

        DCHECK_EQ(codec_context_->codec_type, AVMEDIA_TYPE_AUDIO);
        return true;
    }

    bool AudioFileReader::OpenDecoder() {
        const AVCodec* codec = avcodec_find_decoder(codec_context_->codec_id);
        if (codec) {
            // MP3 decodes to S16P which we don't support, tell it to use S16 instead.
            if (codec_context_->sample_fmt == AV_SAMPLE_FMT_S16P)
                codec_context_->request_sample_fmt = AV_SAMPLE_FMT_S16;

            const int result = avcodec_open2(codec_context_.get(), codec, nullptr);
            if (result < 0) {
                DLOG(WARNING) << "AudioFileReader::Open() : could not open codec -"
                              << " result: " << result;
                return false;
            }

            // Ensure avcodec_open2() respected our format request.
            if (codec_context_->sample_fmt == AV_SAMPLE_FMT_S16P) {
                DLOG(ERROR) << "AudioFileReader::Open() : unable to configure a"
                            << " supported sample format - "
                            << codec_context_->sample_fmt;
                return false;
            }
        } else {
            DLOG(WARNING) << "AudioFileReader::Open() : could not find codec.";
            return false;
        }

        channels_ = codec_context_->channels;
        audio_codec_ = codec_context_->codec_id;
        sample_rate_ = codec_context_->sample_rate;
        av_sample_format_ = codec_context_->sample_fmt;
        return true;
    }

    bool AudioFileReader::ReadPacket(AVPacket* output_packet) {
        while (av_read_frame(glue_->format_context(), output_packet) >= 0) {
            // Skip packets from other streams.
            if (output_packet->stream_index != stream_index_) {
                av_packet_unref(output_packet);
                continue;
            }
            return true;
        }
        return false;
    }

    bool AudioFileReader::OnNewFrame(
            int* total_frames,
            std::vector<std::unique_ptr<AudioBus>>* decoded_audio_packets,
            AVFrame* frame) {
        int frames_read = frame->nb_samples;
        if (frames_read < 0)
            return false;

        const int channels = frame->channels;
        if (frame->sample_rate != sample_rate_ || channels != channels_ ||
            frame->format != av_sample_format_) {
            DLOG(ERROR) << "Unsupported midstream configuration change!"
                        << " Sample Rate: " << frame->sample_rate << " vs "
                        << sample_rate_ << ", Channels: " << channels << " vs "
                        << channels_ << ", Sample Format: " << frame->format << " vs "
                        << av_sample_format_;

            // This is an unrecoverable error, so bail out.  We'll return
            // whatever we've decoded up to this point.
            return false;
        }

        // AAC decoding doesn't properly trim the last packet in a stream, so if we
        // have duration information, use it to set the correct length to avoid extra
        // silence from being output. In the case where we are also discarding some
        // portion of the packet (as indicated by a negative pts), we further want to
        // adjust the duration downward by however much exists before zero.
        if (audio_codec_ == AV_CODEC_ID_AAC && frame->pkt_duration) {
            const int64_t pkt_duration = ConvertFromTimeBase(
                    glue_->format_context()->streams[stream_index_]->time_base,
                    frame->pkt_duration + (std::min)(static_cast<int64_t>(0), frame->pts));
            const int64_t frame_duration =
                    int64_t((frames_read / static_cast<double>(sample_rate_)) * Time::kMicrosecondsPerSecond);

            if (pkt_duration < frame_duration && pkt_duration > 0) {
                const int new_frames_read =
                        (std::floor)(frames_read * (pkt_duration / frame_duration));
                DVLOG(2) << "Shrinking AAC frame from " << frames_read << " to "
                         << new_frames_read << " based on packet duration.";
                frames_read = new_frames_read;

                // The above process may delete the entire packet.
                if (!frames_read)
                    return true;
            }
        }

        // De-interleave each channel and convert to 32bit floating-point with
        // nominal range -1.0 -> +1.0.  If the output is already in float planar
        // format, just copy it into the AudioBus.
        decoded_audio_packets->emplace_back(AudioBus::Create(channels, frames_read));
        AudioBus* audio_bus = decoded_audio_packets->back().get();

        if (codec_context_->sample_fmt == AV_SAMPLE_FMT_FLT) {
            audio_bus->fromInterleaved<Float32SampleTypeTraits>(
                    reinterpret_cast<float*>(frame->data[0]), frames_read);
        } else if (codec_context_->sample_fmt == AV_SAMPLE_FMT_FLTP) {
            for (int ch = 0; ch < audio_bus->channels(); ++ch) {
                memcpy(audio_bus->channel(ch), frame->extended_data[ch],
                       sizeof(float) * frames_read);
            }
        } else {
            int bytes_per_sample = av_get_bytes_per_sample(codec_context_->sample_fmt);
            switch (bytes_per_sample) {
                case 1:
                    audio_bus->fromInterleaved<UnsignedInt8SampleTypeTraits>(
                            reinterpret_cast<const uint8_t*>(frame->data[0]), frames_read);
                    break;
                case 2:
                    audio_bus->fromInterleaved<SignedInt16SampleTypeTraits>(
                            reinterpret_cast<const int16_t*>(frame->data[0]), frames_read);
                    break;
                case 4:
                    audio_bus->fromInterleaved<SignedInt32SampleTypeTraits>(
                            reinterpret_cast<const int32_t*>(frame->data[0]), frames_read);
                    break;
                default:
                    DCHECK(false) << "Unsupported bytes per sample encountered: "
                                  << bytes_per_sample;
                    audio_bus->zeroFrames(frames_read);
            }
        }

        (*total_frames) += frames_read;
        return true;
    }
}
