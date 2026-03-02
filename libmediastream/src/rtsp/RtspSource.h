#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file RtspSource.h
/// @brief RTSP 取流实现 [仅头文件，待从现有 MediaService 迁移]
///
/// 依赖:
/// - FFmpeg (libavformat, libavcodec)   TODO: 需要安装 FFmpeg 开发库

#include "core/IStreamSource.h"
#include "codec/H264Decoder.h"

#include <string>
#include <thread>
#include <atomic>
#include <iostream>
// FFmpeg 前向声明
struct AVFormatContext;
struct AVCodecContext;

namespace ms {

class RtspSource : public IStreamSource {
public:
    RtspSource();
    ~RtspSource() override;

    // IStreamSource 接口
    int  open(const std::string& url, const Options& opts) override;
    int  start() override;
    void stop() override;

private:
    /// 打开 RTSP 流
    int  openStream();

    /// 释放流资源
    void freeStream();

    /// 取流工作线程
    void workerLoop();

private:
    AVFormatContext* fmtCtx_   = nullptr;
    AVCodecContext*  codecCtx_ = nullptr;
    H264Decoder      decoder_;
    int              videoStreamIdx_ = -1;

    std::string      url_;
    Options          options_;
    std::thread      worker_;
    std::atomic<bool> running_{false};
};

} // namespace ms

