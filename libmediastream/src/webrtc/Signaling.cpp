/// @file Signaling.cpp
/// @brief WebSocket 信令客户端完整实现 [Phase 2.1]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "webrtc/Signaling.h"

// TODO: 启用 libdatachannel 后取消注释
// #include <rtc/rtc.hpp>

// TODO: 启用 nlohmann/json 后取消注释
// #include <nlohmann/json.hpp>
// using json = nlohmann::json;

#include <iostream>
#include <sstream>

namespace ms {

Signaling::Signaling() = default;

Signaling::~Signaling()
{
    disconnect();
}

int Signaling::connect(const std::string& url, const std::string& room)
{
    url_  = url;
    room_ = room;

    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    try {
        ws_ = std::make_shared<rtc::WebSocket>();

        ws_->onOpen([this]() {
            connected_ = true;
            std::cout << "[Signaling] WebSocket 已连接: " << url_ << std::endl;

            // 加入房间：发送 join 消息
            json msg;
            msg["type"] = "join";
            msg["room"] = room_;
            sendJson(msg.dump());

            if (onConnected) {
                onConnected();
            }
        });

        ws_->onClosed([this]() {
            connected_ = false;
            std::cout << "[Signaling] WebSocket 已断开" << std::endl;
            if (onDisconnected) {
                onDisconnected();
            }
        });

        ws_->onError([this](const std::string& error) {
            std::cerr << "[Signaling] WebSocket 错误: " << error << std::endl;
        });

        ws_->onMessage([this](auto data) {
            if (std::holds_alternative<std::string>(data)) {
                onMessage(std::get<std::string>(data));
            }
        });

        ws_->open(url);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[Signaling] 连接失败: " << e.what() << std::endl;
        return -1;
    }
    */

    std::cout << "[Signaling] connect() 占位 — 等待 libdatachannel 集成" << std::endl;
    return -1;  // 未实现
}

void Signaling::disconnect()
{
    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    if (ws_) {
        if (ws_->isOpen()) {
            ws_->close();
        }
        ws_.reset();
    }
    */
    connected_ = false;
}

void Signaling::sendAnswer(const std::string& sdp)
{
    // TODO: 启用 nlohmann/json 后取消以下注释
    /*
    json msg;
    msg["type"]     = "answer";
    msg["sdp"]      = {{"type", "answer"}, {"sdp", sdp}};
    sendJson(msg.dump());
    */
    (void)sdp;
    std::cout << "[Signaling] sendAnswer() 占位" << std::endl;
}

void Signaling::sendIceCandidate(const std::string& candidate,
                                  const std::string& sdpMid,
                                  int sdpMLineIndex)
{
    // TODO: 启用 nlohmann/json 后取消以下注释
    /*
    json msg;
    msg["type"]      = "ice-candidate";
    msg["candidate"] = {
        {"candidate",     candidate},
        {"sdpMid",        sdpMid},
        {"sdpMLineIndex", sdpMLineIndex}
    };
    sendJson(msg.dump());
    */
    (void)candidate;
    (void)sdpMid;
    (void)sdpMLineIndex;
    std::cout << "[Signaling] sendIceCandidate() 占位" << std::endl;
}

bool Signaling::isConnected() const
{
    return connected_.load();
}

void Signaling::onMessage(const std::string& message)
{
    // TODO: 启用 nlohmann/json 后取消以下注释
    /*
    try {
        auto msg = json::parse(message);
        std::string type = msg.value("type", "");

        if (type == "offer") {
            // 收到 SDP Offer
            std::string sdpStr = msg["sdp"].value("sdp", "");
            std::string sdpType = msg["sdp"].value("type", "offer");
            if (onOffer) {
                onOffer(sdpType, sdpStr);
            }
        } else if (type == "ice-candidate") {
            // 收到 ICE Candidate
            auto cand = msg["candidate"];
            std::string candidate = cand.value("candidate", "");
            std::string sdpMid    = cand.value("sdpMid", "");
            int sdpMLineIndex     = cand.value("sdpMLineIndex", 0);
            if (onIceCandidate) {
                onIceCandidate(candidate, sdpMid, sdpMLineIndex);
            }
        } else if (type == "peer-joined") {
            std::string peerId = msg.value("peerId", "");
            if (onPeerJoined) {
                onPeerJoined(peerId);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Signaling] JSON 解析错误: " << e.what() << std::endl;
    }
    */
    (void)message;
}

void Signaling::sendJson(const std::string& json)
{
    // TODO: 启用 libdatachannel 后取消以下注释
    /*
    std::lock_guard<std::mutex> lock(sendMutex_);
    if (ws_ && ws_->isOpen()) {
        ws_->send(json);
    }
    */
    (void)json;
}

} // namespace ms

