//
// Created by wang rl on 2022/6/9.
//

/**
 * 变速算法
 *
 * 给定一块音频内存，对某一段进行变速处理
 */

#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
    assert(argc == 2);
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;

    std::string filePath = argv[1];
    size_t bufferSize = std::filesystem::file_size(filePath.c_str());
    LOG(INFO) << "fileSize " << bufferSize;

    auto* bufferData = (uint8_t*) calloc(bufferSize, sizeof(uint8_t));
    if (!bufferData) {
        LOG(ERROR) << "Calloc buffer failed";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}