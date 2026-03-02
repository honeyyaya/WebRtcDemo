#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file Signaling.h
/// @brief WebSocket 信令客户端 [Phase 2.1 完整实现]
///
/// 基于 libdatachannel 内置的 rtc::WebSocket 连接信令服务器，
/// 使用 JSON 收发 join/offer/answer/ice-candidate 消息。
///
/// 依赖:
/// - libdatachannel (rtc::WebSocket)   TODO: 需要引入 libdatachannel v0.21+
/// - nlohmann/json                      TODO: 需要引入 nlohmann/json (header-only)

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>

// TODO: 启用 libdatachannel 后取消以下前向声明，改为 #include <rtc/rtc.hpp>
namespace rtc {
class WebSocket;
}

namespace ms {

/// WebSocket 信令客户端
/// 负责与信令服务器（Node.js）通信，交换 SDP Offer/Answer 和 ICE Candidate
class Signaling {
public:
    Signaling();
    ~Signaling();

    // 禁止拷贝
    Signaling(const Signaling&) = delete;
    Signaling& operator=(const Signaling&) = delete;

    /// 连接信令服务器并加入指定房间
    /// @param url   WebSocket URL, 例如 "ws://192.168.1.100:3000/ws"
    /// @param room  房间名, 例如 "video-room"
    /// @return 0 成功，非 0 失败
    int connect(const std::string& url, const std::string& room);

    /// 断开与信令服务器的连接
    void disconnect();

    /// 发送 SDP Answer 给信令服务器
    void sendAnswer(const std::string& sdp);

    /// 发送 ICE Candidate 给信令服务器
    void sendIceCandidate(const std::string& candidate,
                          const std::string& sdpMid,
                          int sdpMLineIndex);

    /// 是否已连接
    bool isConnected() const;

    // ========== 回调 ==========

    /// 收到远端 SDP Offer 时触发
    /// 参数: type, sdp
    std::function<void(const std::string& type, const std::string& sdp)> onOffer;

    /// 收到远端 ICE Candidate 时触发
    /// 参数: candidate, sdpMid, sdpMLineIndex
    std::function<void(const std::string& candidate,
                       const std::string& sdpMid,
                       int sdpMLineIndex)> onIceCandidate;

    /// 连接成功时触发
    std::function<void()> onConnected;

    /// 连接断开时触发
    std::function<void()> onDisconnected;

    /// 对端加入房间时触发
    std::function<void(const std::string& peerId)> onPeerJoined;

private:
    /// 处理收到的 WebSocket 消息 (JSON)
    void onMessage(const std::string& message);

    /// 通过 WebSocket 发送 JSON 消息
    void sendJson(const std::string& json);

    std::shared_ptr<rtc::WebSocket> ws_;
    std::string                     url_;
    std::string                     room_;
    std::atomic<bool>               connected_{false};
    std::mutex                      sendMutex_;
};

} // namespace ms

