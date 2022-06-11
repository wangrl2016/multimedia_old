//
// Created by wang rl on 2022/6/9.
//

/**
 * 变速算法: 给定一块音频内存，对某一段进行变速处理
 *
 * 参考文档: https://ffmpeg.org/doxygen/trunk/filter_audio_8c-example.html
 */

#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <glog/logging.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
}

/**
 * 对音频进行变速处理
 *
 * @param srcData [out] 原始数据
 * @param srcSize [out] 原始数据大小，byte为单位
 * @param tempo 变速大小，范围为[0.5, 100.0]
 * @param channelCount 原始数据声道数
 * @param sampleRate 原始数据采样率
 * @param sampleFormat 原始数据采样率
 * @return 是否变速成功
 */
bool timeStretch(void** srcData,
                 size_t& srcSize,
                 float tempo = 1.0,
                 int64_t channelLayout = AV_CH_LAYOUT_MONO,
                 int sampleRate = 16000,
                 AVSampleFormat sampleFormat = AV_SAMPLE_FMT_S16) {
    // Set up the filter graph.
    // The filter chain it uses is:
    // (input) -> abuffer -> atempo -> aformat -> abuffersink -> (output)
    // abuffer: This provides the endpoint where you can feed the decoded samples.
    // atempo: The filter accepts exactly one parameter, the audio tempo.
    // If not specified then the filter will assume nominal 1.0 tempo.
    // Tempo must be in the [0.5, 100.0] range.
    // aformat: This converts the samples to the sample freq, channel layout,
    // and sample format required by the audio device.
    // abuffersink: This provides the endpoint where you can read the samples after
    // they have passed through the filter chain.
    if (tempo < 0.5 || tempo > 100) {
        LOG(ERROR) << "Invalid tempo param, tempo " << tempo;
        return false;
    }
    AVFilterGraph* filterGraph = nullptr;
    const AVFilter* abuffer;
    AVFilterContext* abufferCtx = nullptr;
    const AVFilter* atempo;
    AVFilterContext* atempoCtx = nullptr;
    const AVFilter* aformat;
    AVFilterContext* aformatCtx = nullptr;
    const AVFilter* abuffersink;
    AVFilterContext* abuffersinkCtx = nullptr;

    auto clearFFmpegFilters = [&abufferCtx, &atempoCtx,
                  &aformatCtx, &abuffersinkCtx, &filterGraph]() ->void {
        if (abufferCtx)
            avfilter_free(abufferCtx);
        if (atempoCtx)
            avfilter_free(atempoCtx);
        if (aformatCtx)
            avfilter_free(aformatCtx);
        if (abuffersinkCtx)
            avfilter_free(abuffersinkCtx);
        if (filterGraph) {
            avfilter_graph_free(&filterGraph);
        }
    };

    // Create a new filter graph, which will contain all the filters.
    filterGraph = avfilter_graph_alloc();
    if (!filterGraph) {
        LOG(ERROR) << "Unable to create filter graph";
        clearFFmpegFilters();
        return false;
    }

    // Create the abuffer filter;
    // it will be used for feeding the data into the graph.
    abuffer = avfilter_get_by_name("abuffer");
    if (!abuffer) {
        LOG(ERROR) << "Could not find the abuffer filter";
        clearFFmpegFilters();
        return false;
    }
    abufferCtx = avfilter_graph_alloc_filter(filterGraph, abuffer, "src");
    if (!abufferCtx) {
        LOG(ERROR) << "Could not allocate the abuffer instance";
        clearFFmpegFilters();
        return false;
    }
    // Set the filter options through the AVOptions API.
    char chLayoutStr[64];
    av_get_channel_layout_string(chLayoutStr,
                                 sizeof(chLayoutStr),
                                 av_get_channel_layout_nb_channels(channelLayout),
                                 channelLayout);
    av_opt_set(abufferCtx, "channel_layout", chLayoutStr, AV_OPT_SEARCH_CHILDREN);
    av_opt_set(abufferCtx,
               "sample_fmt",
               av_get_sample_fmt_name(sampleFormat),
               AV_OPT_SEARCH_CHILDREN);
    av_opt_set_q(abufferCtx,
                 "time_base",
                 (AVRational) {1, sampleRate},
                 AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abufferCtx, "sample_rate", sampleRate, AV_OPT_SEARCH_CHILDREN);
    // Now initialize the filter; we pass NULL options, since we have already
    // set all the options above.
    int err = avfilter_init_str(abufferCtx, nullptr);
    if (err < 0) {
        LOG(ERROR) << "Could not initialize the abuffer filter";
        clearFFmpegFilters();
        return false;
    }

    // Create atempo filter
    atempo = avfilter_get_by_name("atempo");
    if (!atempo) {
        LOG(ERROR) << "Could not find the atempo filter";
        clearFFmpegFilters();
        return false;
    }
    atempoCtx = avfilter_graph_alloc_filter(filterGraph, atempo, "atempo");
    if (!atempoCtx) {
        LOG(ERROR) << "Could not allocate the atempo instance";
        clearFFmpegFilters();
        return false;
    }
    // A different way of passing the options is as key/value pairs in a dictionary.
    AVDictionary* optionsDict = nullptr;
    av_dict_set(&optionsDict, "tempo", std::to_string(tempo).c_str(), 0);
    err = avfilter_init_dict(atempoCtx, &optionsDict);
    av_dict_free(&optionsDict);
    if (err < 0) {
        LOG(ERROR) << "Could not initialize the atempo filter";
        clearFFmpegFilters();
        return false;
    }

    // Create the aformat filter.
    // It ensures that the output is of the format we want.
    aformat = avfilter_get_by_name("aformat");
    if (!aformat) {
        LOG(ERROR) << "Could not find the aformat filter";
        clearFFmpegFilters();
        return false;
    }
    aformatCtx = avfilter_graph_alloc_filter(filterGraph, aformat, "aformat");
    if (!aformatCtx) {
        LOG(ERROR) << "Could not allocate the aformat instance";
        clearFFmpegFilters();
        return false;
    }
    // A third way of passing the options is in a string of the form
    // key1=value1:key2=value2...
    uint8_t optionsStr[1024];
    // 指定输出为单声道
    snprintf((char*) optionsStr, sizeof(optionsStr),
             "sample_fmts=%s:sample_rates=%d:channel_layouts=mono",
             av_get_sample_fmt_name(sampleFormat), sampleRate);
    err = avfilter_init_str(aformatCtx, (char*) optionsStr);
    if (err < 0) {
        LOG(ERROR) << "Could not initialize the aformat filter";
        clearFFmpegFilters();
        return false;
    }

    // Finally, create the abuffersink filter;
    // it will be used to get the filtered data out of the graph.
    abuffersink = avfilter_get_by_name("abuffersink");
    if (!abuffersink) {
        LOG(ERROR) << "Could not find the abuffersink filter";
        clearFFmpegFilters();
        return false;
    }
    abuffersinkCtx = avfilter_graph_alloc_filter(filterGraph, abuffersink, "sink");
    if (!abuffersinkCtx) {
        LOG(ERROR) << "Could not allocate the abuffersink instance";
        clearFFmpegFilters();
        return false;
    }
    // This filter takes no options.
    err = avfilter_init_str(abuffersinkCtx, nullptr);
    if (err < 0) {
        LOG(ERROR) << "Could not initialize the abuffersink instance";
        clearFFmpegFilters();
        return false;
    }

    // Connect the filters;
    // in this simple case the filters just form a linear chain.
    err = avfilter_link(abufferCtx, 0, atempoCtx, 0);
    if (err >= 0)
        err = avfilter_link(atempoCtx, 0, aformatCtx, 0);
    else {
        LOG(ERROR) << "Error connecting filters";
        clearFFmpegFilters();
        return false;
    }
    if (err >= 0)
        err = avfilter_link(aformatCtx, 0, abuffersinkCtx, 0);
    if (err < 0) {
        LOG(ERROR) << "Error connecting filters";
        clearFFmpegFilters();
        return false;
    }

    // Configure the graph.
    err = avfilter_graph_config(filterGraph, nullptr);
    if (err < 0) {
        LOG(ERROR) << "Error configuring the filter graph";
        clearFFmpegFilters();
        return false;
    }

    // 分配内存存储生成的数据，比实际需要的要大
    // 没有使用vector的目的是避免频繁分配
    auto destSize = int(float(srcSize) / tempo + 8192);
    void* destData = calloc(destSize, sizeof(uint8_t));
    if (!destData) {
        LOG(ERROR) << "Error allocating buffer";
        // 释放结构体
        clearFFmpegFilters();
        return false;
    }

    // Allocate the frame we will be using to store the data.
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        LOG(ERROR) << "Error allocating the frame";
        free(destData);
        clearFFmpegFilters();
        return false;
    }

    int srcIndex = 0; // 记录目前处理过的原始数据下标，以byte为单位
    int destIndex = 0; // 记录目前生成的目标数据下标，以byte为单位
    int bytesPerSample = av_get_bytes_per_sample(sampleFormat);

    // 避免代码重复
    auto consumeFrame = [&abuffersinkCtx, &frame,
                         &destIndex, &destSize, &destData, &bytesPerSample]() ->void {
        while (av_buffersink_get_frame(abuffersinkCtx, frame) >= 0) {
            int frameSize = frame->nb_samples * bytesPerSample;
            // 只处理第0声道的数据
            if (destIndex + frameSize > destSize) {
                // 说明内存分配够不大，或者是哪里有问题
                LOG(WARNING) << "Malloc buffer small";
                av_frame_unref(frame);
                break;
            }
            memcpy(((char*)destData) + destIndex, frame->extended_data[0], frameSize);
            destIndex += frameSize;
            av_frame_unref(frame);
        }
    };

    bool flushed = false;
    while (srcIndex < srcSize && !flushed) {
        frame->sample_rate = sampleRate;
        frame->format = sampleFormat;
        frame->channel_layout = channelLayout;
        frame->channels = 1;
        frame->nb_samples = 1024;
        // 存在srcSize - srcIndex < frame->nb_samples * bytesPerSamples的情况
        if (srcSize - srcIndex < frame->nb_samples * bytesPerSample) {
            flushed = true;
            int samplesLeft = int(srcSize - srcIndex) / bytesPerSample;
            frame->nb_samples = samplesLeft;
        }
        err = av_frame_get_buffer(frame, 0);
        if (err < 0) {
            LOG(ERROR) << "Error frame get buffer";
            free(destData);
            av_frame_free(&frame);
            clearFFmpegFilters();
            return false;
        }

        // 将数据复制进入frame中
        memcpy(frame->extended_data[0],
               ((char*)(*srcData)) + srcIndex,
               frame->nb_samples * bytesPerSample);
        srcIndex += frame->nb_samples * bytesPerSample;

        // Send the frame to the input of the filter graph.
        err = av_buffersrc_add_frame(abufferCtx, frame);
        if (err < 0) {
            av_frame_unref(frame);
            LOG(ERROR) << "Error submitting the frame to the filter graph";
            continue;
        }

        // Get all the filtered output that is available.
        consumeFrame();
    }

    // flush
    err = av_buffersrc_add_frame(abufferCtx, nullptr);
    if (err >= 0) {
        consumeFrame();
    }

    // 将数据传到原数据中
    if ((*srcData)) {
        free(*srcData);
    }

    (*srcData) = destData;
    srcSize = destIndex;

    clearFFmpegFilters();
    av_frame_free(&frame);
    return true;
}

int main(int argc, char* argv[]) {
    assert(argc == 2);
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;

    size_t bufferSize = std::filesystem::file_size(argv[1]);  // byte为单位
    LOG(INFO) << "bufferSize " << bufferSize << " bytes";
    void* bufferData = calloc(bufferSize, sizeof(uint8_t));

    std::ifstream ifs;
    ifs.open(argv[1], std::ios::binary);
    ifs.read((char*) bufferData, (long) bufferSize);

    float tempo = 0.85;
    bool ret = timeStretch(&bufferData, bufferSize, tempo);
    if (!ret) {
        LOG(WARNING) << "Time stretch failed: tempo " << tempo;
    }
    LOG(INFO) << "destSize " << bufferSize << " bytes";

    std::ofstream ofs("out/time_stretch.pcm", std::ios::out | std::ios::binary);
    ofs.write((const char*) bufferData, (long) bufferSize);

    ifs.close();
    ofs.close();

    return EXIT_SUCCESS;
}
