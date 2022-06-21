cmake_minimum_required(VERSION 3.15)

project(multimedia VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_parties)

add_library(multimedia STATIC)
target_sources(multimedia
        PRIVATE
        base/memory/AlignedMemory.cpp
        base/time/Time.cpp
        common/AudioProperties.cpp
        common/FFmpegAudioDecoder.cpp
        common/Utilities.cpp
        media/base/AudioBus.cpp
        media/ffmpeg/FFmpegCommon.cpp
        media/filters/AudioFileReader.cpp
        media/filters/FFmpegGlue.cpp
        )

set(EXAMPLES
        audio/AudioUnitPlayer.cpp
        audio/DemuxDecode.cpp
        audio/TimeStretchEffect.cpp
        audio/WavFormat.cpp
        )

foreach(EXAMPLE ${EXAMPLES})
    string(REGEX MATCHALL "[0-9A-Za-z_]*.cpp" tmp1 ${EXAMPLE})
    string(REGEX REPLACE ".cpp" "" EXE ${tmp1})
    add_executable(${EXE}
            ${EXAMPLE})
    target_link_libraries(${EXE}
            multimedia
            ${CONAN_LIBS})
endforeach()

# 单元测试
enable_testing()

add_executable(MMUnitTest
        tests/AudioBusUnitTest.cpp)

target_link_libraries(MMUnitTest
        PRIVATE
        multimedia
        ${CONAN_LIBS})

include(GoogleTest)
gtest_discover_tests(MMUnitTest)
