//
// Created by wang rl on 2022/6/13.
//

#ifndef MULTIMEDIA_TIME_H
#define MULTIMEDIA_TIME_H

#include <cstdint>

namespace mm {
    // Provides value storage and comparison/math operations common to all time
    // classes. Each subclass provides for strong type-checking to ensure
    // semantically meaningful comparison/math of time values from the same clock
    // source or timeline.
    template<class TimeClass>
    class TimeBase {
    public:
        static constexpr int64_t kHoursPerDay = 24;
        static constexpr int64_t kSecondsPerMinute = 60;
        static constexpr int64_t kMinutesPerHour = 60;
        static constexpr int64_t kSecondsPerHour =
                kSecondsPerMinute * kMinutesPerHour;
        static constexpr int64_t kMillisecondsPerSecond = 1000;
        static constexpr int64_t kMillisecondsPerDay =
                kMillisecondsPerSecond * kSecondsPerHour * kHoursPerDay;
        static constexpr int64_t kMicrosecondsPerMillisecond = 1000;
        static constexpr int64_t kMicrosecondsPerSecond =
                kMicrosecondsPerMillisecond * kMillisecondsPerSecond;
        static constexpr int64_t kMicrosecondsPerMinute =
                kMicrosecondsPerSecond * kSecondsPerMinute;
        static constexpr int64_t kMicrosecondsPerHour =
                kMicrosecondsPerMinute * kMinutesPerHour;
        static constexpr int64_t kMicrosecondsPerDay =
                kMicrosecondsPerHour * kHoursPerDay;
        static constexpr int64_t kMicrosecondsPerWeek = kMicrosecondsPerDay * 7;
        static constexpr int64_t kNanosecondsPerMicrosecond = 1000;
        static constexpr int64_t kNanosecondsPerSecond =
                kNanosecondsPerMicrosecond * kMicrosecondsPerSecond;
    };

    class Time : public TimeBase<Time> {

    };
}

#endif //MULTIMEDIA_TIME_H
