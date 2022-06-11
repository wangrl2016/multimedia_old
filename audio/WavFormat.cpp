//
// Created by wang rl on 2022/6/11.
//

#include <cassert>
#include <filesystem>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
    assert(argc == 2);
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;

    size_t fileSize = std::filesystem::file_size(argv[1]);
    LOG(INFO) << "fileSize " << fileSize;

    size_t bufferSize = fileSize * sizeof(int16_t);
    auto* bufferData = (uint8_t*) calloc(bufferSize, sizeof(uint8_t));

    return EXIT_SUCCESS;
}