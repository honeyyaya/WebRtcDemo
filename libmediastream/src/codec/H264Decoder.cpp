/// @file H264Decoder.cpp
/// @brief FFmpeg H.264 解码器封装完整实现 [Phase 2.1]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "codec/H264Decoder.h"

// TODO: 启用 FFmpeg 后取消以下注释
// extern "C" {
// #include <libavcodec/avcodec.h>
// #include <libavutil/imgutils.h>
// #include <libavutil/mem.h>
// }

#include <iostream>
#include <cstring>

namespace ms {

H264Decoder::H264Decoder() = default;

H264Decoder::~H264Decoder()
{
    close();
}

int H264Decoder::open()
{
    if (open_) return 0;

    // TODO: 启用 FFmpeg 后取消以下注释
    /*
    // 查找 H.264 解码器
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "[H264Decoder] 找不到 H.264 解码器" << std::endl;
        return -1;
    }

    // 创建解码器上下文
    ctx_ = avcodec_alloc_context3(codec);
    if (!ctx_) {
        std::cerr << "[H264Decoder] 无法分配解码器上下文" << std::endl;
        return -2;
    }

    // 设置低延迟解码选项
    ctx_->flags  |= AV_CODEC_FLAG_LOW_DELAY;
    ctx_->flags2 |= AV_CODEC_FLAG2_FAST;

    // 允许不完整帧输入（网络流可能有丢包）
    if (codec->capabilities & AV_CODEC_CAP_TRUNCATED) {
        ctx_->flags |= AV_CODEC_FLAG_TRUNCATED;
    }

    // 打开解码器
    int ret = avcodec_open2(ctx_, codec, nullptr);
    if (ret < 0) {
        std::cerr << "[H264Decoder] 无法打开解码器, ret=" << ret << std::endl;
        avcodec_free_context(&ctx_);
        return -3;
    }

    // 分配帧和包
    frame_ = av_frame_alloc();
    if (!frame_) {
        avcodec_free_context(&ctx_);
        return -4;
    }

    pkt_ = av_packet_alloc();
    if (!pkt_) {
        av_frame_free(&frame_);
        avcodec_free_context(&ctx_);
        return -5;
    }

    open_ = true;
    std::cout << "[H264Decoder] 解码器已初始化" << std::endl;
    return 0;
    */

    std::cout << "[H264Decoder] open() 占位 — 等待 FFmpeg 集成" << std::endl;
    open_ = true;  // 标记为已打开以便流程能走通
    return 0;
}

void H264Decoder::close()
{
    if (!open_) return;

    // TODO: 启用 FFmpeg 后取消以下注释
    /*
    if (pkt_) {
        av_packet_free(&pkt_);
        pkt_ = nullptr;
    }
    if (frame_) {
        av_frame_free(&frame_);
        frame_ = nullptr;
    }
    if (ctx_) {
        avcodec_free_context(&ctx_);
        ctx_ = nullptr;
    }
    */

    open_ = false;
    std::cout << "[H264Decoder] 解码器已关闭" << std::endl;
}

bool H264Decoder::isOpen() const
{
    return open_;
}

int H264Decoder::decode(const uint8_t* data, int size, YuvFrame& outFrame)
{
    if (!open_) return -1;
    if (!data || size <= 0) return -2;

    // TODO: 启用 FFmpeg 后取消以下注释
    /*
    // 设置输入数据
    pkt_->data = const_cast<uint8_t*>(data);
    pkt_->size = size;

    // 送入解码器
    int ret = avcodec_send_packet(ctx_, pkt_);
    if (ret < 0) {
        if (ret == AVERROR(EAGAIN)) {
            // 解码器内部缓冲满，需要先取出帧
        } else if (ret == AVERROR_EOF) {
            return -3;
        } else {
            std::cerr << "[H264Decoder] avcodec_send_packet 失败, ret=" << ret << std::endl;
            return -4;
        }
    }

    // 尝试获取解码后的帧
    ret = avcodec_receive_frame(ctx_, frame_);
    if (ret == AVERROR(EAGAIN)) {
        // 需要更多数据
        return 1;
    } else if (ret == AVERROR_EOF) {
        return -3;
    } else if (ret < 0) {
        std::cerr << "[H264Decoder] avcodec_receive_frame 失败, ret=" << ret << std::endl;
        return -5;
    }

    // 解码成功，填充输出帧
    outFrame.y        = frame_->data[0];
    outFrame.u        = frame_->data[1];
    outFrame.v        = frame_->data[2];
    outFrame.width    = frame_->width;
    outFrame.height   = frame_->height;
    outFrame.yStride  = frame_->linesize[0];
    outFrame.uvStride = frame_->linesize[1];
    outFrame.ptsMs    = frame_->pts;  // 需要根据 time_base 转换

    return 0;
    */

    // 占位：FFmpeg 未集成，返回"需要更多数据"
    (void)data;
    (void)size;
    (void)outFrame;
    return 1;
}

} // namespace ms

