cmake_minimum_required(VERSION 3.15)

project(multimedia VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_parties)

set(EXAMPLES
        audio/TimeStretchEffect.cpp
        audio/WavFormat.cpp
        )

foreach(EXAMPLE ${EXAMPLES})
    string(REGEX MATCHALL "[0-9A-Za-z_]*.cpp" tmp1 ${EXAMPLE})
    string(REGEX REPLACE ".cpp" "" EXE ${tmp1})
    add_executable(${EXE}
            ${EXAMPLE})
    target_sources(
            ${EXE}
            PRIVATE
            common/Utilities.cpp
    )
    target_link_libraries(${EXE}
            ${CONAN_LIBS})
endforeach()
