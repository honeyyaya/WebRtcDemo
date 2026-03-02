/// @file RtspSource.cpp
/// @brief RTSP 取流实现 [空桩 — 待从现有 MediaService 迁移]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "rtsp/RtspSource.h"
#include <iostream>
// TODO: 启用 FFmpeg 后取消注释
// extern "C" {
// #include <libavformat/avformat.h>
// #include <libavcodec/avcodec.h>
// }

#include <iostream>

namespace ms {

RtspSource::RtspSource() = default;

RtspSource::~RtspSource()
{
    stop();
}

int RtspSource::open(const std::string& url, const Options& opts)
{
    url_ = url;
    options_ = opts;

    // TODO: 从现有 MediaService 迁移
    // 1. avformat_open_input()
    // 2. avformat_find_stream_info()
    // 3. 找到视频流索引
    // 4. 初始化 H264Decoder
    std::cout << "[RtspSource] open() 空桩 — 待实现" << std::endl;
    return -1;  // 未实现
}

int RtspSource::start()
{
    // TODO: 从现有 MediaService 迁移
    // 启动 workerLoop 线程

    std::cout << "[RtspSource] start() 空桩 — 待实现" << std::endl;
    return -1;  // 未实现
}

void RtspSource::stop()
{
    if (!running_.load()) return;
    running_ = false;

    if (worker_.joinable()) {
        worker_.join();
    }

    freeStream();
}

int RtspSource::openStream()
{
    // TODO: 从现有 MediaService.openFfmpegStream() 迁移
    return -1;
}

void RtspSource::freeStream()
{
    // TODO: 从现有 MediaService.freeFfmpegCtx() 迁移
    decoder_.close();
}

void RtspSource::workerLoop()
{
    // TODO: 从现有 MediaService.demuxing() 迁移
    // loop:
    //   av_read_frame()
    //   decoder_.decode()
    //   onFrame(yuvFrame)
}

} // namespace ms

