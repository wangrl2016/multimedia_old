//
// Created by WangRuiLing on 2022/6/13.
//

#include <cmath>
#include <glog/logging.h>
#include "common/FFmpegAudioDecoder.h"
#include "media/ffmpeg/FFmpegCommon.h"

namespace mm {
    // AAC(M4A) decoding specific constants.
    static const int kAACPrimingFrameCount = 2112;
    static const int kAACRemainderFrameCount = 519;

    bool FFmpegAudioDecoder::open(const std::string &filePath) {
        // Open input file, and allocate format context.
        if (avformat_open_input(&mFormatCtx, filePath.c_str(), nullptr, nullptr) < 0) {
            LOG(ERROR) << "Could not open source file";
            this->close();
            return false;
        }

        // Retrieve stream information.
        if (avformat_find_stream_info(mFormatCtx, nullptr) < 0) {
            LOG(ERROR) << "Could not find stream information";
            this->close();
            return false;
        }

        mStreamIndex = av_find_best_stream(mFormatCtx,
                                           AVMEDIA_TYPE_AUDIO,
                                           -1,
                                           -1,
                                           &mCodec, 0);
        if (mStreamIndex < 0) {
            LOG(ERROR) << "Could not find stream in input file";
            this->close();
            return false;
        }

        // Allocate a codec context for the decoder.
        mCodecCtx = avcodec_alloc_context3(mCodec);
        if (!mCodecCtx) {
            LOG(ERROR) << "Failed to allocate the codec context";
            this->close();
            return false;
        }

        AVStream* stream = mFormatCtx->streams[mStreamIndex];
        mCodecCtx->pkt_timebase = stream->time_base;
        // Copy codec parameters from input stream to output codec context.
        if (avcodec_parameters_to_context(mCodecCtx, stream->codecpar) < 0) {
            LOG(ERROR) << "Failed to copy codec parameters to decoder context";
            this->close();
            return false;
        }

        // Init the decoders.
        if (avcodec_open2(mCodecCtx, mCodec, nullptr) < 0) {
            LOG(ERROR) << "Failed to open codec";
            this->close();
            return false;
        }

        // Store initial values to guard against midstream configuration changes.
        mSrcAudioProperties.channelCount = av_get_channel_layout_nb_channels(
                mCodecCtx->channel_layout);
        mSrcAudioProperties.sampleRate = mCodecCtx->sample_rate;
        mSrcAudioProperties.sampleFormat = mCodecCtx->sample_fmt;

        return true;
    }

    bool FFmpegAudioDecoder::hasKnownDuration() const {
        return mFormatCtx->duration != AV_NOPTS_VALUE;
    }

    int64_t FFmpegAudioDecoder::getDuration() const {
        const AVRational avTimeBase = {1, AV_TIME_BASE};
        DCHECK_NE(mFormatCtx->duration, AV_NOPTS_VALUE);
        int64_t estimatedDurationUs = mFormatCtx->duration;

        if (mCodecCtx->codec_id == AV_CODEC_ID_AAC) {
            // For certain AAC-encoded files, FFmpeg estimated frame count might not
            // be sufficient to capture the entire audio content that we want. This is
            // especially noticeable for short files (< 10ms) resulting in silence
            // throughout the decoded buffer. Thus, we add the priming frames and the
            // remainder frames to the estimation.
            estimatedDurationUs += ceil(
                    1000000.0 *
                    static_cast<double>(kAACPrimingFrameCount + kAACRemainderFrameCount) /
                    srcSampleRate());
        } else {
            // Add one microsecond to avoid rounding-down errors which can occur when
            // |duration| has been calculated from an exact number of sample-frames.
            // One microsecond is much less than the time of a single sample-frame
            // at any real-world sample-rate.
            estimatedDurationUs += 1;
        }

        return ConvertFromTimeBase(avTimeBase, estimatedDurationUs);
    }

    int FFmpegAudioDecoder::getNumberOfFrames() const {
        return std::ceil(double(getDuration())  / 1000000.0 / srcSampleRate());
    }

    void FFmpegAudioDecoder::close() {
        if (mFormatCtx) {
            avformat_close_input(&mFormatCtx);
            mFormatCtx = nullptr;
        }
        if (mCodecCtx) {
            avcodec_free_context(&mCodecCtx);
            mCodecCtx = nullptr;
        }
        if (mSwrCtx) {
            swr_free(&mSwrCtx);
            mSwrCtx = nullptr;
        }
    }
}
