//
// Created by WangRuiLing on 2022/6/21.
//

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#include "media/ffmpeg/ffmpeg_deleters.h"

namespace mm {
    // The following implement the deleters declared in ffmpeg_deleters.h.
    void ScopedPtrAVFree::operator()(void* x) const {
        av_free(x);
    }

    void ScopedPtrAVFreePacket::operator()(void* x) const {
        AVPacket* packet = static_cast<AVPacket*>(x);
        av_packet_free(&packet);
    }

    void ScopedPtrAVFreeContext::operator()(void* x) const {
        AVCodecContext* codec_context = static_cast<AVCodecContext*>(x);
        avcodec_free_context(&codec_context);
    }

    void ScopedPtrAVFreeFrame::operator()(void* x) const {
        AVFrame* frame = static_cast<AVFrame*>(x);
        av_frame_free(&frame);
    }
}
