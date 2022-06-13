//
// Created by WangRuiLing on 2022/6/13.
//

#include <cassert>
#include <cstdlib>
#include <memory>
#include <glog/logging.h>
#include "common/FFmpegAudioDecoder.h"

int main(int argc, char* argv[]) {
    assert(argc == 2);
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::GLOG_INFO;

    std::shared_ptr<mm::FFmpegAudioDecoder> ffmpegAudioDecoder =
            std::make_shared<mm::FFmpegAudioDecoder>();
    if (!ffmpegAudioDecoder->open(argv[1])) {
        LOG(ERROR) << "Open " << argv[1] << " failed";
        return EXIT_FAILURE;
    }

    mm::AudioProperties audioProperties = ffmpegAudioDecoder->getSrcAudioProperties();
    audioProperties.dump();

    return EXIT_SUCCESS;
}