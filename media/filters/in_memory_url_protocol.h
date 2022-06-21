//
// Created by WangRuiLing on 2022/6/21.
//

#ifndef MULTIMEDIA_IN_MEMORY_URL_PROTOCOL_H
#define MULTIMEDIA_IN_MEMORY_URL_PROTOCOL_H

#include "media/filters/ffmpeg_glue.h"

namespace mm {
    // Simple FFmpegURLProtocol that reads from a buffer.
    // NOTE: This object does not copy the buffer so the
    //       buffer pointer passed into the constructor
    //       needs to remain valid for the entire lifetime of
    //       this object.
    class InMemoryUrlProtocol : public FFmpegURLProtocol {
    public:
        InMemoryUrlProtocol() = delete;

        InMemoryUrlProtocol(const uint8_t* buf, int64_t size, bool streaming);

        InMemoryUrlProtocol(const InMemoryUrlProtocol&) = delete;

        InMemoryUrlProtocol& operator=(const InMemoryUrlProtocol&) = delete;

        virtual ~InMemoryUrlProtocol();

        // FFmpegURLProtocol methods.
        int Read(int size, uint8_t* data) override;

        bool GetPosition(int64_t* position_out) override;

        bool SetPosition(int64_t position) override;

        bool GetSize(int64_t* size_out) override;

    private:
        const uint8_t* data_;
        int64_t size_;
        int64_t position_;
        bool streaming_;
    };

} // mm

#endif //MULTIMEDIA_IN_MEMORY_URL_PROTOCOL_H
