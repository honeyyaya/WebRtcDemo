#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file RtcpFeedback.h
/// @brief RTCP 反馈模块 [Phase 2.3/2.4 — 仅头文件，待实现]
///
/// 功能：
/// - NACK 重传请求 (RFC 4585)        [Phase 2.3]
/// - PLI 关键帧请求 (RFC 4585)        [Phase 2.4]
/// - FIR 关键帧请求 (RFC 5104)        [Phase 2.4]
/// - REMB 拥塞反馈 (draft-alvestrand) [Phase 2.6]
///
/// 依赖:
/// - libdatachannel (rtc::Track)   TODO: 需要引入 libdatachannel v0.21+

#include <cstdint>
#include <vector>
#include <chrono>
#include <memory>
// TODO: 启用 libdatachannel 后取消以下前向声明
namespace rtc {
class Track;
}

namespace ms {

class RtcpFeedback {
public:
    RtcpFeedback();
    ~RtcpFeedback();

    /// 设置关联的媒体轨道（用于发送 RTCP 包）
    void setTrack(std::shared_ptr<rtc::Track> track);

    /// 设置 SSRC
    void setSsrc(uint32_t localSsrc, uint32_t remoteSsrc);

    // ========== NACK (Phase 2.3) ==========

    /// 发送 NACK 请求重传指定序列号的包
    /// @param seqList 丢失的 RTP 序列号列表
    void sendNack(const std::vector<uint16_t>& seqList);

    // ========== PLI / FIR (Phase 2.4) ==========

    /// 发送 PLI (Picture Loss Indication) 请求关键帧
    void sendPLI();

    /// 发送 FIR (Full Intra Request) 强制关键帧
    void sendFIR();

    // ========== REMB (Phase 2.6) ==========

    /// 发送 REMB (Receiver Estimated Maximum Bitrate) 拥塞反馈
    /// @param bitrateBps 估计的可用带宽 (bps)
    void sendREMB(uint32_t bitrateBps);

    /// 设置 PLI/FIR 最小发送间隔（防止过于频繁）
    void setMinPliInterval(int ms);
    void setMinFirInterval(int ms);

private:
    // ========== RTCP 包构建 ==========

    std::vector<uint8_t> buildNackPacket(const std::vector<uint16_t>& seqList);
    std::vector<uint8_t> buildPliPacket();
    std::vector<uint8_t> buildFirPacket();
    std::vector<uint8_t> buildRembPacket(uint32_t bitrateBps);

    /// 通过 Track 发送 RTCP 包
    void sendRtcp(const std::vector<uint8_t>& packet);

private:
    std::shared_ptr<rtc::Track> track_;
    uint32_t localSsrc_  = 0;
    uint32_t remoteSsrc_ = 0;
    uint8_t  firSeqNum_  = 0;  // FIR 序列号，每次递增

    // 频率限制
    int minPliIntervalMs_ = 1000;
    int minFirIntervalMs_ = 1000;
    int minRembIntervalMs_ = 1000;

    std::chrono::steady_clock::time_point lastPliTime_;
    std::chrono::steady_clock::time_point lastFirTime_;
    std::chrono::steady_clock::time_point lastRembTime_;
};

} // namespace ms

