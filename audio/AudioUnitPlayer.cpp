//
// Created by wang rl on 2022/6/12.
//

/**
 * 使用AudioUnit进行声音播放
 */

#include <cstdlib>
#ifdef __APPLE__
#include <AudioUnit/AudioUnit.h>
#include <filesystem>
#include <fstream>
#include <glog/logging.h>

#include "common/Utilities.h"

size_t bufferSize = 0;

float* destData = nullptr;

uint32_t playIndex = 0;

bool quit = false;

OSStatus renderCallbackFunc(void* inRefCon,
                            AudioUnitRenderActionFlags* ioActionFlags,
                            const AudioTimeStamp* inTimeStamp,
                            UInt32 inBusNumber,
                            UInt32 inNumberFrames,
                            AudioBufferList* ioData) {
    float* out = (float*) ioData->mBuffers[0].mData;
    memcpy(out, (float*)destData + playIndex, inNumberFrames * sizeof(float));
    playIndex += inNumberFrames;
    return noErr;
}
#endif

int main(int argc, char* argv[]) {
#ifdef __APPLE__
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;

    bufferSize = std::filesystem::file_size(argv[1]);  // byte为单位
    LOG(INFO) << "bufferSize " << bufferSize << " bytes";
    void* bufferData = calloc(bufferSize, sizeof(uint8_t));
    destData = (float*) calloc(bufferSize / sizeof(int16_t), sizeof(float));

    std::ifstream ifs;
    ifs.open(argv[1], std::ios::binary);
    ifs.read((char*) bufferData, (long) bufferSize);

    mm::convertPcm16ToFloat((const int16_t*)bufferData,
                            destData,
                            bufferSize / sizeof(int16_t));

    OSStatus status;
    AudioUnit audioUnit;
    AudioComponentDescription acd;
    acd.componentType = kAudioUnitType_Output;
    acd.componentSubType = kAudioUnitSubType_DefaultOutput;
    acd.componentFlags = 0;
    acd.componentFlagsMask = 0;
    acd.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent component = AudioComponentFindNext(nullptr, &acd);
    status = AudioComponentInstanceNew(component, &audioUnit);

    UInt32 maxFramePerSlice = 4096;
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &maxFramePerSlice,
                                  sizeof(UInt32));

    UInt32 flag = 1;
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output,
                                  0,
                                  &flag,
                                  sizeof(UInt32));

    AudioStreamBasicDescription asbd;
    memset(&asbd, 0, sizeof(asbd));
    int byteSize = sizeof(Float32);
    asbd.mSampleRate = 16000;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
    asbd.mChannelsPerFrame = 1;
    asbd.mBytesPerPacket = byteSize;
    asbd.mBytesPerFrame = byteSize;
    asbd.mFramesPerPacket = 1;
    asbd.mBitsPerChannel = 8 * byteSize;
    asbd.mReserved = 0;

    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &asbd,
                                  sizeof(AudioStreamBasicDescription));

    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = renderCallbackFunc;
    callbackStruct.inputProcRefCon = nullptr;
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global,
                                  0,
                                  &callbackStruct,
                                  sizeof(AURenderCallbackStruct));

    status = AudioUnitInitialize(audioUnit);

    // 新开线程进行播放
    status = AudioOutputUnitStart(audioUnit);

    LOG(INFO) << "exit";
    sleep(50000);
#endif
    return EXIT_SUCCESS;
}
