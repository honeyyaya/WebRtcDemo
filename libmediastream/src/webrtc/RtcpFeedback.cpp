/// @file RtcpFeedback.cpp
/// @brief RTCP 反馈模块 [Phase 2.3/2.4 — 空桩，待实现]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "webrtc/RtcpFeedback.h"

// TODO: 启用 libdatachannel 后取消注释
// #include <rtc/rtc.hpp>

#include <iostream>

namespace ms {

RtcpFeedback::RtcpFeedback()
    : lastPliTime_(std::chrono::steady_clock::now())
    , lastFirTime_(std::chrono::steady_clock::now())
    , lastRembTime_(std::chrono::steady_clock::now())
{
}

RtcpFeedback::~RtcpFeedback() = default;

void RtcpFeedback::setTrack(std::shared_ptr<rtc::Track> track)
{
    track_ = track;
}

void RtcpFeedback::setSsrc(uint32_t localSsrc, uint32_t remoteSsrc)
{
    localSsrc_  = localSsrc;
    remoteSsrc_ = remoteSsrc;
}

void RtcpFeedback::sendNack(const std::vector<uint16_t>& /*seqList*/)
{
    // TODO: Phase 2.3 实现
    // auto packet = buildNackPacket(seqList);
    // sendRtcp(packet);
}

void RtcpFeedback::sendPLI()
{
    // TODO: Phase 2.4 实现
    // auto packet = buildPliPacket();
    // sendRtcp(packet);
}

void RtcpFeedback::sendFIR()
{
    // TODO: Phase 2.4 实现
    // auto packet = buildFirPacket();
    // sendRtcp(packet);
}

void RtcpFeedback::sendREMB(uint32_t /*bitrateBps*/)
{
    // TODO: Phase 2.6 实现
    // auto packet = buildRembPacket(bitrateBps);
    // sendRtcp(packet);
}

void RtcpFeedback::setMinPliInterval(int ms) { minPliIntervalMs_ = ms; }
void RtcpFeedback::setMinFirInterval(int ms) { minFirIntervalMs_ = ms; }

std::vector<uint8_t> RtcpFeedback::buildNackPacket(const std::vector<uint16_t>& /*seqList*/)
{
    // TODO: Phase 2.3 实现 — 构建 RTCP NACK (RFC 4585, FMT=1, PT=205)
    return {};
}

std::vector<uint8_t> RtcpFeedback::buildPliPacket()
{
    // TODO: Phase 2.4 实现 — 构建 RTCP PLI (RFC 4585, FMT=1, PT=206)
    return {};
}

std::vector<uint8_t> RtcpFeedback::buildFirPacket()
{
    // TODO: Phase 2.4 实现 — 构建 RTCP FIR (RFC 5104, FMT=4, PT=206)
    return {};
}

std::vector<uint8_t> RtcpFeedback::buildRembPacket(uint32_t /*bitrateBps*/)
{
    // TODO: Phase 2.6 实现 — 构建 RTCP REMB (draft-alvestrand, FMT=15, PT=206)
    return {};
}

void RtcpFeedback::sendRtcp(const std::vector<uint8_t>& /*packet*/)
{
    // TODO: 启用 libdatachannel 后实现
    // if (track_) { track_->send(packet); }
}

} // namespace ms

