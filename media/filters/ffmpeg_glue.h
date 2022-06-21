//
// Created by WangRuiLing on 2022/6/20.
//

// FFmpegGlue is an interface  read and seek requests to internal data structures.
// The glue works through the AVIO interface provided by FFmpeg.
//
// AVIO works through a special AVIOContext created through avio_alloc_context()
// which is attached to the AVFormatContext used for de-muxing.  The AVIO context
// is initialized with read and seek methods which FFmpeg calls when necessary.
//
// During OpenContext() FFmpegGlue will tell FFmpeg to use Chrome's AVIO context
// by passing NULL in for the filename parameter to avformat_open_input().  All
// FFmpeg operations using the configured AVFormatContext will then redirect
// reads and seeks through the glue.
//
// The glue in turn processes those read and seek requests using the
// FFmpegURLProtocol provided during construction.

#ifndef MULTIMEDIA_FFMPEG_GLUE_H
#define MULTIMEDIA_FFMPEG_GLUE_H

#include <cstdint>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
}

#include "media/ffmpeg/ffmpeg_common.h"

namespace mm {
    class FFmpegURLProtocol {
    public:
        // Read the given amount of bytes into data, returns the number of bytes read
        // if successful, kReadError otherwise.
        virtual int Read(int size, uint8_t* data) = 0;

        // Returns true and the current file position for this file, false if the
        // file position could not be retrieved.
        virtual bool GetPosition(int64_t* position_out) = 0;

        // Returns true if the file position could be set, false otherwise.
        virtual bool SetPosition(int64_t position) = 0;

        // Returns true and the file size, false if the file size could not be
        // retrieved.
        virtual bool GetSize(int64_t* size_out) = 0;
    };

    class FFmpegGlue {
    public:
        // See file documentation for usage.  |protocol| must outlive FFmpegGlue.
        explicit FFmpegGlue(FFmpegURLProtocol* protocol);

        FFmpegGlue(const FFmpegGlue&) = delete;

        FFmpegGlue& operator=(const FFmpegGlue&) = delete;

        ~FFmpegGlue();

        // Opens an AVFormatContext specially prepared to process reads and seeks
        // through the FFmpegURLProtocol provided during construction.
        // |is_local_file| is an optional parameter used for metrics reporting.
        bool OpenContext(bool is_local_file = false);

        AVFormatContext* format_context() { return format_context_; }

    private:
        bool open_called_ = false;
        AVFormatContext* format_context_ = nullptr;
        std::unique_ptr<AVIOContext, ScopedPtrAVFree> avio_context_;
    };
}

#endif //MULTIMEDIA_FFMPEG_GLUE_H
