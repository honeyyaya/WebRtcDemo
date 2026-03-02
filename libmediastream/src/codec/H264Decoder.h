#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file H264Decoder.h
/// @brief FFmpeg H.264 解码器封装 [Phase 2.1 完整实现]
///
/// 封装 FFmpeg avcodec_send_packet / avcodec_receive_frame，
/// 供 RtspSource 和 WebRtcSource 共用。
///
/// 依赖:
/// - FFmpeg (libavcodec, libavutil)   TODO: 需要安装 FFmpeg 开发库

#include "core/Types.h"
#include <cstdint>

// FFmpeg 前向声明，避免暴露 FFmpeg 头文件
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

namespace ms {

class H264Decoder {
public:
    H264Decoder();
    ~H264Decoder();

    // 禁止拷贝
    H264Decoder(const H264Decoder&) = delete;
    H264Decoder& operator=(const H264Decoder&) = delete;

    /// 初始化解码器
    /// @return 0 成功，非 0 失败
    int  open();

    /// 关闭解码器，释放资源
    void close();

    /// 是否已初始化
    bool isOpen() const;

    /// 解码一个 NAL Unit / Access Unit
    /// @param data     H.264 数据（可含起始码 00 00 00 01）
    /// @param size     数据长度
    /// @param outFrame [out] 解码成功时填充 YUV 帧数据
    /// @return 0 成功解码出一帧，1 需要更多数据，<0 错误
    int  decode(const uint8_t* data, int size, YuvFrame& outFrame);

private:
    AVCodecContext* ctx_    = nullptr;
    AVFrame*        frame_  = nullptr;
    AVPacket*       pkt_    = nullptr;
    bool            open_   = false;
};

} // namespace ms

