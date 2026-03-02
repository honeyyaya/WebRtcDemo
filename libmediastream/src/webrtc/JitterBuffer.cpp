/// @file JitterBuffer.cpp
/// @brief 简版抖动缓冲 [Phase 2.2 — 空桩，待实现]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "webrtc/JitterBuffer.h"

namespace ms {

JitterBuffer::JitterBuffer(const Config& config)
    : config_(config)
    , targetDelayMs_(config.initialDelayMs)
{
}

JitterBuffer::~JitterBuffer() = default;

void JitterBuffer::insert(const RtpPacket& /*packet*/)
{
    // TODO: Phase 2.2 实现
    // 1. 按 seq 插入 buffer_
    // 2. 更新 jitter 估计
    // 3. 检测丢包空洞
}

std::optional<RtpPacket> JitterBuffer::poll()
{
    // TODO: Phase 2.2 实现
    // 1. 检查 nextSeqOut_ 是否在 buffer_ 中
    // 2. 判断延迟是否满足 targetDelayMs_
    // 3. 按序输出
    return std::nullopt;
}

std::vector<uint16_t> JitterBuffer::missingSeqs() const
{
    // TODO: Phase 2.2 实现
    // 扫描 buffer_ 中 nextSeqOut_ 到最大 seq 之间的空洞
    return {};
}

void JitterBuffer::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
    nextSeqOut_ = 0;
    hasFirstSeq_ = false;
    jitterEstimate_ = 0.0;
    hasLastTs_ = false;
    stats_ = Stats{};
}

JitterBuffer::Stats JitterBuffer::stats() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void JitterBuffer::setConfig(const Config& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}

void JitterBuffer::updateJitter(uint32_t /*rtpTimestamp*/, int64_t /*arrivalTimeMs*/)
{
    // TODO: Phase 2.2 实现
    // RFC 3550 抖动估计算法
}

} // namespace ms

