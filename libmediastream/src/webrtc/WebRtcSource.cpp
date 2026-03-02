/// @file WebRtcSource.cpp
/// @brief WebRTC 取流实现 [Phase 2.1]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "webrtc/WebRtcSource.h"
#include "core/Types.h"

// TODO: 启用 libdatachannel 后取消注释
// #include <rtc/rtc.hpp>

#include <iostream>
#include <chrono>

namespace ms {

WebRtcSource::WebRtcSource() = default;

WebRtcSource::~WebRtcSource()
{
    stop();
}

int WebRtcSource::open(const std::string& url, const Options& opts)
{
    signalingUrl_ = url;

    // 从 options 中获取房间名
    auto it = opts.find("room");
    room_ = (it != opts.end()) ? it->second : "video-room";

    // 设置信令回调
    signaling_.onOffer = [this](const std::string& type, const std::string& sdp) {
        handleOffer(type, sdp);
    };

    signaling_.onIceCandidate = [this](const std::string& candidate,
                                       const std::string& sdpMid,
                                       int sdpMLineIndex) {
        handleRemoteCandidate(candidate, sdpMid, sdpMLineIndex);
    };

    signaling_.onConnected = [this]() {
        notifyState(StreamState::Connecting, "信令已连接，等待 Offer");
    };

    signaling_.onDisconnected = [this]() {
        notifyState(StreamState::Error, "信令断开连接");
    };

    // 设置 RTP 解包回调 —— 当一个完整 NAL Unit 组装好时
    depacketizer_.onNalUnit = [this](std::vector<uint8_t> nalUnit) {
        std::lock_guard<std::mutex> lock(nalMutex_);
        nalQueue_.push(std::move(nalUnit));
    };

    // 初始化解码器
    int ret = decoder_.open();
    if (ret != 0) {
        std::cout <<"[WebRtcSource] H264Decoder 初始化失败"<< std::endl;
        return -1;
    }

    return 0;
}

int WebRtcSource::start()
{
    if (running_.load()) return 0;
    running_ = true;

    // 连接信令服务器
    int ret = signaling_.connect(signalingUrl_, room_);
    if (ret != 0) {
        std::cerr << "[WebRtcSource] 信令连接失败" << std::endl;
        running_ = false;
        return -1;
    }

    // 初始化 PeerConnection
    setupPeerConnection();

    // 启动解码工作线程
    decodeWorker_ = std::thread([this]() { decodeLoop(); });

    notifyState(StreamState::Connecting, "正在连接...");
    return 0;
}

void WebRtcSource::stop()
{
    if (!running_.load()) return;
    running_ = false;

    // 唤醒解码线程
    rtpCv_.notify_all();

    // 等待解码线程结束
    if (decodeWorker_.joinable()) {
        decodeWorker_.join();
    }

    // 断开信令
    signaling_.disconnect();

    // 关闭 PeerConnection
    // TODO: 启用 libdatachannel 后取消注释
    /*
    if (pc_) {
        pc_->close();
        pc_.reset();
    }
    */

    videoTrack_.reset();

    // 关闭解码器
    decoder_.close();

    // 清空队列
    {
        std::lock_guard<std::mutex> lock(rtpMutex_);
        while (!rtpQueue_.empty()) rtpQueue_.pop();
    }
    {
        std::lock_guard<std::mutex> lock(nalMutex_);
        while (!nalQueue_.empty()) nalQueue_.pop();
    }

    notifyState(StreamState::Stopped);
}

void WebRtcSource::setupPeerConnection()
{
    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    pc_ = std::make_shared<rtc::PeerConnection>(config);

    pc_->onStateChange([this](rtc::PeerConnection::State state) {
        std::cout << "[WebRtcSource] PeerConnection 状态: "
                  << static_cast<int>(state) << std::endl;
        if (state == rtc::PeerConnection::State::Connected) {
            notifyState(StreamState::Playing, "WebRTC 已连接");
        } else if (state == rtc::PeerConnection::State::Failed) {
            notifyState(StreamState::Error, "PeerConnection 连接失败");
        } else if (state == rtc::PeerConnection::State::Disconnected) {
            notifyState(StreamState::Error, "PeerConnection 断开");
        }
    });

    pc_->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
        std::cout << "[WebRtcSource] ICE gathering 状态: "
                  << static_cast<int>(state) << std::endl;
    });

    pc_->onLocalCandidate([this](rtc::Candidate candidate) {
        // 将本地 ICE Candidate 通过信令发送给远端
        signaling_.sendIceCandidate(
            std::string(candidate),
            candidate.mid(),
            0  // sdpMLineIndex
        );
    });

    pc_->onTrack([this](std::shared_ptr<rtc::Track> track) {
        onTrack(track);
    });
    */

    std::cout << "[WebRtcSource] setupPeerConnection() 占位 — 等待 libdatachannel 集成"
              << std::endl;
}

void WebRtcSource::handleOffer(const std::string& type, const std::string& sdp)
{
    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    if (!pc_) return;

    pc_->setRemoteDescription(rtc::Description(sdp, type));

    // 创建 Answer
    pc_->setLocalDescription(rtc::Description::Type::Answer);

    // 获取本地 SDP 并发送
    auto localDesc = pc_->localDescription();
    if (localDesc) {
        signaling_.sendAnswer(std::string(*localDesc));
    }
    */
    (void)type;
    (void)sdp;
    std::cout << "[WebRtcSource] handleOffer() 占位" << std::endl;
}

void WebRtcSource::handleRemoteCandidate(const std::string& candidate,
                                         const std::string& sdpMid,
                                         int sdpMLineIndex)
{
    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    if (!pc_) return;
    pc_->addRemoteCandidate(rtc::Candidate(candidate, sdpMid));
    */
    (void)candidate;
    (void)sdpMid;
    (void)sdpMLineIndex;
}

void WebRtcSource::onTrack(std::shared_ptr<rtc::Track> track)
{
    std::cout << "[WebRtcSource] 收到视频轨道" << std::endl;
    videoTrack_ = track;

    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    track->onMessage([this](rtc::message_variant data) {
        if (std::holds_alternative<rtc::binary>(data)) {
            auto& bin = std::get<rtc::binary>(data);
            onRtpPacket(reinterpret_cast<const uint8_t*>(bin.data()), bin.size());
        }
    }, nullptr);
    */
}

void WebRtcSource::onRtpPacket(const uint8_t* data, size_t len)
{
    // 将 RTP 包投递到解码线程队列
    std::vector<uint8_t> packet(data, data + len);
    {
        std::lock_guard<std::mutex> lock(rtpMutex_);
        rtpQueue_.push(std::move(packet));
    }
    rtpCv_.notify_one();
}

void WebRtcSource::decodeLoop()
{
    std::cout << "[WebRtcSource] 解码线程启动" << std::endl;

    while (running_.load()) {
        // 1. 等待 RTP 包
        std::vector<uint8_t> rtpPacket;
        {
            std::unique_lock<std::mutex> lock(rtpMutex_);
            rtpCv_.wait_for(lock, std::chrono::milliseconds(50),
                            [this] { return !rtpQueue_.empty() || !running_.load(); });

            if (!running_.load()) break;
            if (rtpQueue_.empty()) continue;

            rtpPacket = std::move(rtpQueue_.front());
            rtpQueue_.pop();
        }

        // 2. 送入 RTP 解包器
        depacketizer_.feed(rtpPacket.data(), rtpPacket.size());

        // 3. 检查是否有完整的 NAL Unit 可以解码
        std::vector<uint8_t> nalUnit;
        {
            std::lock_guard<std::mutex> lock(nalMutex_);
            if (nalQueue_.empty()) continue;
            nalUnit = std::move(nalQueue_.front());
            nalQueue_.pop();
        }

        // 4. 解码 NAL Unit
        YuvFrame frame;
        int ret = decoder_.decode(nalUnit.data(),
                                  static_cast<int>(nalUnit.size()),
                                  frame);
        if (ret == 0) {
            // 5. 回调输出帧
            if (onFrame) {
                onFrame(frame);
            }
        } else {
            // 解码失败，后续可以在此触发 PLI 请求关键帧 (Phase 2.4)
            std::cerr << "[WebRtcSource] 解码失败 ret=" << ret << std::endl;
        }
    }

    std::cout << "[WebRtcSource] 解码线程退出" << std::endl;
}

void WebRtcSource::notifyState(StreamState state, const std::string& msg)
{
    if (onState) {
        onState(state, msg);
    }
}

} // namespace ms

