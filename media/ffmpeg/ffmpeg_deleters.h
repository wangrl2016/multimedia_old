//
// Created by WangRuiLing on 2022/6/21.
//

#ifndef MULTIMEDIA_FFMPEG_DELETER_H
#define MULTIMEDIA_FFMPEG_DELETER_H

namespace mm {
    // Wraps FFmpeg's av_free() in a class that can be passed as a template argument
    // to scoped_ptr_malloc.
    struct ScopedPtrAVFree {
        void operator()(void* x) const;
    };

    // Calls av_packet_free(). Do not use this with an AVPacket instance that was
    // allocated with new or manually av_malloc'd. ScopedAVPacket is the
    // recommended way to manage an AVPacket's lifetime.
    struct ScopedPtrAVFreePacket {
        void operator()(void* x) const;
    };

    // Frees an AVCodecContext object in a class that can be passed as a Deleter
    // argument to scoped_ptr_malloc.
    struct ScopedPtrAVFreeContext {
        void operator()(void* x) const;
    };

    // Frees an AVFrame object in a class that can be passed as a Deleter argument
    // to scoped_ptr_malloc.
    struct ScopedPtrAVFreeFrame {
        void operator()(void* x) const;
    };
}

#endif //MULTIMEDIA_FFMPEG_DELETER_H
