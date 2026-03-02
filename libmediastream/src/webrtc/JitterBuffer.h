#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file JitterBuffer.h
/// @brief 简版抖动缓冲 [Phase 2.2 — 仅头文件，待实现]
///
/// 功能：
/// 1. 按 RTP 序列号排序乱序包
/// 2. 按时间戳 (RTP timestamp) 组帧
/// 3. 提供固定/自适应延迟缓冲，平滑网络抖动
/// 4. 检测丢包，提供缺失序列号列表（供 NACK 使用）

#include "core/Types.h"

#include <cstdint>
#include <vector>
#include <map>
#include <optional>
#include <mutex>

namespace ms {

class JitterBuffer {
public:
    /// 配置参数
    struct Config {
        int minDelayMs      = 20;    ///< 最小缓冲延迟 (ms)
        int maxDelayMs      = 100;   ///< 最大缓冲延迟 (ms)
        int initialDelayMs  = 50;    ///< 初始缓冲延迟 (ms)
        int maxPacketCount  = 512;   ///< 缓冲区最大包数
    };

    /// 统计信息
    struct Stats {
        uint32_t currentDelayMs   = 0;
        uint32_t packetsReceived  = 0;
        uint32_t packetsLost      = 0;
        uint32_t framesDecoded    = 0;
        uint32_t framesDropped    = 0;
        float    jitterMs         = 0.0f;
    };

    explicit JitterBuffer(const Config& config = Config{});
    ~JitterBuffer();

    /// 插入一个 RTP 包
    void insert(const RtpPacket& packet);

    /// 按序取出一个 RTP 包（如果有序可用）
    std::optional<RtpPacket> poll();

    /// 获取当前缺失的序列号列表（用于 NACK 请求）
    std::vector<uint16_t> missingSeqs() const;

    /// 重置缓冲区
    void reset();

    /// 获取统计信息
    Stats stats() const;

    /// 更新配置
    void setConfig(const Config& config);

private:
    /// 更新抖动估计
    void updateJitter(uint32_t rtpTimestamp, int64_t arrivalTimeMs);

    Config                          config_;
    mutable std::mutex              mutex_;
    std::map<uint16_t, RtpPacket>   buffer_;     ///< 按序列号排序的包缓冲
    uint16_t                        nextSeqOut_ = 0;
    bool                            hasFirstSeq_ = false;

    // 抖动估计
    double   jitterEstimate_ = 0.0;
    int      targetDelayMs_  = 50;
    uint32_t lastRtpTs_      = 0;
    int64_t  lastArrivalMs_  = 0;
    bool     hasLastTs_      = false;

    // 统计
    Stats stats_;
};

} // namespace ms

