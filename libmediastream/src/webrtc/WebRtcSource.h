#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file WebRtcSource.h
/// @brief WebRTC 取流实现 [Phase 2.1 完整实现]
///
/// 创建 rtc::PeerConnection，配置 STUN/ICE，
/// 处理 onTrack 回调，获取 RTP 数据，
/// 内部启动解码线程：RtpDepacketizer -> H264Decoder -> onFrame
///
/// 依赖:
/// - libdatachannel   TODO: 需要引入 libdatachannel v0.21+
/// - FFmpeg           TODO: 需要安装 FFmpeg 开发库

#include "core/IStreamSource.h"
#include "webrtc/Signaling.h"
#include "webrtc/RtpDepacketizer.h"
#include "codec/H264Decoder.h"

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>

// TODO: 启用 libdatachannel 后取消以下前向声明，改为 #include <rtc/rtc.hpp>
namespace rtc {
class PeerConnection;
class Track;
}

namespace ms {

/// WebRTC 取流源
/// 实现 IStreamSource 接口，通过 libdatachannel 接收 WebRTC 视频流
class WebRtcSource : public IStreamSource {
public:
    WebRtcSource();
    ~WebRtcSource() override;

    // IStreamSource 接口
    int  open(const std::string& url, const Options& opts) override;
    int  start() override;
    void stop() override;

private:
    /// 初始化 PeerConnection 及其回调
    void setupPeerConnection();

    /// 处理收到的 SDP Offer
    void handleOffer(const std::string& type, const std::string& sdp);

    /// 处理收到的远端 ICE Candidate
    void handleRemoteCandidate(const std::string& candidate,
                               const std::string& sdpMid,
                               int sdpMLineIndex);

    /// onTrack 回调 —— 当远端媒体轨道可用时触发
    void onTrack(std::shared_ptr<rtc::Track> track);

    /// 收到 RTP 包时的处理
    void onRtpPacket(const uint8_t* data, size_t len);

    /// 解码工作线程
    void decodeLoop();

    /// 通知状态变化
    void notifyState(StreamState state, const std::string& msg = "");

private:
    Signaling        signaling_;
    RtpDepacketizer  depacketizer_;
    H264Decoder      decoder_;

    std::shared_ptr<rtc::PeerConnection> pc_;
    std::shared_ptr<rtc::Track>          videoTrack_;

    // 解码线程
    std::thread       decodeWorker_;
    std::atomic<bool> running_{false};

    // RTP 数据队列：网络线程 → 解码线程
    std::mutex                    rtpMutex_;
    std::condition_variable       rtpCv_;
    std::queue<std::vector<uint8_t>> rtpQueue_;

    // NAL 组帧缓冲：由 RtpDepacketizer 输出
    std::mutex                    nalMutex_;
    std::queue<std::vector<uint8_t>> nalQueue_;

    // 配置
    std::string signalingUrl_;
    std::string room_;
};

} // namespace ms

