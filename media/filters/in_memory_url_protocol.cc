//
// Created by WangRuiLing on 2022/6/21.
//

#include "media/filters/in_memory_url_protocol.h"

namespace mm {
    InMemoryUrlProtocol::InMemoryUrlProtocol(const uint8_t* data,
                                             int64_t size,
                                             bool streaming)
            : data_(data),
              size_(size >= 0 ? size : 0),
              position_(0),
              streaming_(streaming) {}

    InMemoryUrlProtocol::~InMemoryUrlProtocol() = default;

    int InMemoryUrlProtocol::Read(int size, uint8_t* data) {
        // Not sure if this can happen, but it's unclear from the ffmpeg code, so guard
        // against it.
        if (size < 0)
            return AVERROR(EIO);
        if (!size)
            return 0;

        const int64_t available_bytes = size_ - position_;
        if (available_bytes <= 0)
            return AVERROR_EOF;

        if (size > available_bytes)
            size = available_bytes;

        if (size > 0) {
            memcpy(data, data_ + position_, size);
            position_ += size;
        }

        return size;
    }

    bool InMemoryUrlProtocol::GetPosition(int64_t* position_out) {
        if (!position_out)
            return false;

        *position_out = position_;
        return true;
    }

    bool InMemoryUrlProtocol::SetPosition(int64_t position) {
        if (position < 0 || position > size_)
            return false;
        position_ = position;
        return true;
    }

    bool InMemoryUrlProtocol::GetSize(int64_t* size_out) {
        if (!size_out)
            return false;

        *size_out = size_;
        return true;
    }
} // mm