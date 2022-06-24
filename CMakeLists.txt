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
        media/ffmpeg/ffmpeg_common.cc
        media/ffmpeg/ffmpeg_deleters.cc
        media/filters/audio_file_reader.cpp
        media/filters/ffmpeg_glue.cpp
        media/filters/in_memory_url_protocol.cc
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
        tests/audio_bus_unittest.cc
        tests/audio_file_reader_unittest.cc
        tests/in_memory_url_protocol_unittest.cc
        tests/vector_unittest.cc
        )

target_link_libraries(MMUnitTest
        PRIVATE
        multimedia
        ${CONAN_LIBS})

include(GoogleTest)
gtest_discover_tests(MMUnitTest)
