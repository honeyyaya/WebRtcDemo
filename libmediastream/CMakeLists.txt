cmake_minimum_required(VERSION 3.16)
project(mediastream VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 隐藏所有符号，只暴露 MS_API 标记的
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ==============================================================================
# 依赖
# ==============================================================================

# ---- FFmpeg ----
# TODO: FFmpeg not available yet
# 需要安装 FFmpeg 开发库 (libavcodec, libavformat, libavutil, libswscale)
# Windows 预编译下载: https://github.com/BtbN/FFmpeg-Builds/releases
# 取消下面的注释并设置 FFMPEG_ROOT 路径:
#
# set(FFMPEG_ROOT "" CACHE PATH "Path to FFmpeg installation")
# find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h PATHS ${FFMPEG_ROOT}/include)
# find_library(AVCODEC_LIB avcodec PATHS ${FFMPEG_ROOT}/lib)
# find_library(AVFORMAT_LIB avformat PATHS ${FFMPEG_ROOT}/lib)
# find_library(AVUTIL_LIB avutil PATHS ${FFMPEG_ROOT}/lib)
# find_library(SWSCALE_LIB swscale PATHS ${FFMPEG_ROOT}/lib)
# set(FFMPEG_LIBRARIES ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB})
# set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})

# ---- libdatachannel (仅 WebRTC 需要) ----
# TODO: libdatachannel not available yet
# 需要通过 FetchContent 或 git submodule 引入 libdatachannel v0.21+
# 取消下面的注释启用:
#
# include(FetchContent)
# FetchContent_Declare(
#     libdatachannel
#     GIT_REPOSITORY https://github.com/nicedatachannels/libdatachannel.git
#     GIT_TAG        v0.21.1
# )
# option(NO_EXAMPLES "Disable examples" ON)
# option(NO_TESTS "Disable tests" ON)
# FetchContent_MakeAvailable(libdatachannel)

# ---- nlohmann/json ----
# TODO: 需要引入 nlohmann/json (header-only)
# 取消下面的注释启用:
#
# FetchContent_Declare(
#     nlohmann_json
#     GIT_REPOSITORY https://github.com/nlohmann/json.git
#     GIT_TAG        v3.11.3
# )
# FetchContent_MakeAvailable(nlohmann_json)

# ==============================================================================
# WebRTC 可选编译开关
# ==============================================================================
option(ENABLE_WEBRTC "Build with WebRTC support" ON)
if(ENABLE_WEBRTC)
    add_compile_definitions(MS_ENABLE_WEBRTC)
endif()

# ==============================================================================
# 源文件
# ==============================================================================
set(CORE_SOURCES
    src/api.cpp
    src/core/StreamSession.cpp
)

set(RTSP_SOURCES
    src/rtsp/RtspSource.cpp
)

set(WEBRTC_SOURCES
    src/webrtc/WebRtcSource.cpp
    src/webrtc/Signaling.cpp
    src/webrtc/RtpDepacketizer.cpp
    src/webrtc/JitterBuffer.cpp
    src/webrtc/RtcpFeedback.cpp
)

set(CODEC_SOURCES
    src/codec/H264Decoder.cpp
)

set(ALL_SOURCES ${CORE_SOURCES} ${RTSP_SOURCES} ${CODEC_SOURCES})
if(ENABLE_WEBRTC)
    list(APPEND ALL_SOURCES ${WEBRTC_SOURCES})
endif()

# ==============================================================================
# 构建目标
# ==============================================================================
add_library(mediastream SHARED ${ALL_SOURCES})

target_compile_definitions(mediastream PRIVATE MEDIASTREAM_EXPORTS)

target_include_directories(mediastream
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    # PRIVATE ${FFMPEG_INCLUDE_DIRS}          # TODO: 启用 FFmpeg 后取消注释
)

# target_link_libraries(mediastream PRIVATE ${FFMPEG_LIBRARIES})  # TODO: 启用 FFmpeg 后取消注释
# if(ENABLE_WEBRTC)
#     target_link_libraries(mediastream PRIVATE LibDataChannel::LibDataChannel)  # TODO: 启用 libdatachannel 后取消注释
#     target_link_libraries(mediastream PRIVATE nlohmann_json::nlohmann_json)    # TODO: 启用 nlohmann/json 后取消注释
# endif()

# ==============================================================================
# 测试程序
# ==============================================================================
add_executable(mediastream_test test/main.cpp)
target_link_libraries(mediastream_test PRIVATE mediastream)

