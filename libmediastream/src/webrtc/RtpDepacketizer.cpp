/// @file RtpDepacketizer.cpp
/// @brief H.264 RTP 解包器完整实现 [Phase 2.1]

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "webrtc/RtpDepacketizer.h"

#include <cstring>
#include <iostream>

namespace ms {

// H.264 NAL 起始码
static const uint8_t kStartCode[] = { 0x00, 0x00, 0x00, 0x01 };

RtpDepacketizer::RtpDepacketizer() = default;
RtpDepacketizer::~RtpDepacketizer() = default;

void RtpDepacketizer::reset()
{
    fuBuffer_.clear();
    fuActive_ = false;
    fuTimestamp_ = 0;
    auBuffer_.clear();
    currentTimestamp_ = 0;
    hasTimestamp_ = false;
    lastSeq_ = 0;
    hasLastSeq_ = false;
}

bool RtpDepacketizer::parseRtpHeader(const uint8_t* data, size_t len,
                                      RtpHeader& header)
{
    // RTP 头部最小 12 字节
    if (len < 12) return false;

    uint8_t v  = (data[0] >> 6) & 0x03;  // version
    // uint8_t p  = (data[0] >> 5) & 0x01;  // padding
    uint8_t x  = (data[0] >> 4) & 0x01;  // extension
    uint8_t cc = data[0] & 0x0F;          // CSRC count

    if (v != 2) return false;  // 必须是 RTP v2

    header.marker      = (data[1] >> 7) & 0x01;
    header.payloadType = data[1] & 0x7F;
    header.seq         = (static_cast<uint16_t>(data[2]) << 8) | data[3];
    header.timestamp   = (static_cast<uint32_t>(data[4]) << 24) |
                         (static_cast<uint32_t>(data[5]) << 16) |
                         (static_cast<uint32_t>(data[6]) << 8)  |
                         data[7];
    header.ssrc        = (static_cast<uint32_t>(data[8])  << 24) |
                         (static_cast<uint32_t>(data[9])  << 16) |
                         (static_cast<uint32_t>(data[10]) << 8)  |
                         data[11];

    int headerLen = 12 + cc * 4;

    // 处理 RTP 扩展头
    if (x && static_cast<size_t>(headerLen + 4) <= len) {
        // 扩展头长度（以 4 字节为单位）
        uint16_t extLen = (static_cast<uint16_t>(data[headerLen + 2]) << 8) |
                          data[headerLen + 3];
        headerLen += 4 + extLen * 4;
    }

    if (static_cast<size_t>(headerLen) > len) return false;

    header.headerLen = headerLen;
    return true;
}

void RtpDepacketizer::feed(const uint8_t* data, size_t len)
{
    RtpHeader header;
    if (!parseRtpHeader(data, len, header)) {
        std::cerr << "[RtpDepacketizer] RTP 头部解析失败" << std::endl;
        return;
    }

    // 序列号检测（简单丢包检测）
    if (hasLastSeq_) {
        uint16_t expectedSeq = lastSeq_ + 1;
        if (header.seq != expectedSeq) {
            // 可能乱序或丢包，后续由 JitterBuffer 处理 (Phase 2.2)
            // 这里仅记录日志
            int gap = static_cast<int>(header.seq) - static_cast<int>(expectedSeq);
            if (gap > 0 && gap < 1000) {
                std::cerr << "[RtpDepacketizer] 序列号跳跃: expected="
                          << expectedSeq << " got=" << header.seq
                          << " gap=" << gap << std::endl;
            }
        }
    }
    lastSeq_ = header.seq;
    hasLastSeq_ = true;

    // 提取 RTP payload
    const uint8_t* payload = data + header.headerLen;
    size_t payloadLen = len - header.headerLen;

    if (payloadLen < 1) return;

    // NAL 单元头的第一个字节
    uint8_t nalType = payload[0] & 0x1F;

    if (nalType >= 1 && nalType <= 23) {
        // Single NAL Unit
        parseSingleNal(payload, payloadLen, header.timestamp, header.marker);
    } else if (nalType == 24) {
        // STAP-A
        parseStapA(payload, payloadLen, header.timestamp, header.marker);
    } else if (nalType == 28) {
        // FU-A
        parseFuA(payload, payloadLen, header.timestamp, header.marker);
    } else {
        std::cerr << "[RtpDepacketizer] 未支持的 NAL 类型: " << (int)nalType << std::endl;
    }

    // 如果 marker bit 置位，表示一个 Access Unit (帧) 结束
    if (header.marker) {
        flushAccessUnit();
    }
}

void RtpDepacketizer::parseSingleNal(const uint8_t* payload, size_t len,
                                      uint32_t timestamp, bool /*marker*/)
{
    // 检查时间戳变化 —— 新的帧
    if (hasTimestamp_ && timestamp != currentTimestamp_) {
        flushAccessUnit();
    }
    currentTimestamp_ = timestamp;
    hasTimestamp_ = true;

    // 构造带起始码的 NAL Unit
    std::vector<uint8_t> nalUnit;
    nalUnit.reserve(4 + len);
    nalUnit.insert(nalUnit.end(), kStartCode, kStartCode + 4);
    nalUnit.insert(nalUnit.end(), payload, payload + len);
    auBuffer_.push_back(std::move(nalUnit));
}

void RtpDepacketizer::parseFuA(const uint8_t* payload, size_t len,
                                uint32_t timestamp, bool /*marker*/)
{
    if (len < 2) return;

    // FU Indicator: payload[0]
    // FU Header:    payload[1]
    uint8_t fuIndicator = payload[0];
    uint8_t fuHeader    = payload[1];

    bool    startBit = (fuHeader >> 7) & 0x01;
    bool    endBit   = (fuHeader >> 6) & 0x01;
    uint8_t nalType  = fuHeader & 0x1F;

    // 重建 NAL 头字节: F + NRI 来自 FU Indicator, Type 来自 FU Header
    uint8_t nalHeader = (fuIndicator & 0xE0) | nalType;

    if (startBit) {
        // FU-A 起始分片
        fuBuffer_.clear();
        fuBuffer_.insert(fuBuffer_.end(), kStartCode, kStartCode + 4);
        fuBuffer_.push_back(nalHeader);
        fuBuffer_.insert(fuBuffer_.end(), payload + 2, payload + len);
        fuActive_ = true;
        fuTimestamp_ = timestamp;
    } else if (fuActive_) {
        // FU-A 中间或结束分片
        if (timestamp != fuTimestamp_) {
            // 时间戳不匹配，丢弃当前 FU-A
            std::cerr << "[RtpDepacketizer] FU-A 时间戳不匹配，丢弃" << std::endl;
            fuBuffer_.clear();
            fuActive_ = false;
            return;
        }

        fuBuffer_.insert(fuBuffer_.end(), payload + 2, payload + len);

        if (endBit) {
            // FU-A 重组完成
            // 检查时间戳变化
            if (hasTimestamp_ && timestamp != currentTimestamp_) {
                flushAccessUnit();
            }
            currentTimestamp_ = timestamp;
            hasTimestamp_ = true;

            auBuffer_.push_back(std::move(fuBuffer_));
            fuBuffer_.clear();
            fuActive_ = false;
        }
    }
}

void RtpDepacketizer::parseStapA(const uint8_t* payload, size_t len,
                                  uint32_t timestamp, bool /*marker*/)
{
    if (len < 3) return;

    // 检查时间戳变化
    if (hasTimestamp_ && timestamp != currentTimestamp_) {
        flushAccessUnit();
    }
    currentTimestamp_ = timestamp;
    hasTimestamp_ = true;

    // 跳过 STAP-A NAL 头 (1 字节)
    size_t offset = 1;

    while (offset + 2 <= len) {
        // 每个聚合单元: 2 字节长度 + NAL 数据
        uint16_t nalSize = (static_cast<uint16_t>(payload[offset]) << 8) |
                           payload[offset + 1];
        offset += 2;

        if (offset + nalSize > len) {
            std::cerr << "[RtpDepacketizer] STAP-A NAL 长度越界" << std::endl;
            break;
        }

        // 构造带起始码的 NAL Unit
        std::vector<uint8_t> nalUnit;
        nalUnit.reserve(4 + nalSize);
        nalUnit.insert(nalUnit.end(), kStartCode, kStartCode + 4);
        nalUnit.insert(nalUnit.end(), payload + offset, payload + offset + nalSize);
        auBuffer_.push_back(std::move(nalUnit));

        offset += nalSize;
    }
}

void RtpDepacketizer::emitNalUnit(const uint8_t* nalData, size_t nalLen)
{
    if (!onNalUnit || nalLen == 0) return;
    std::vector<uint8_t> data(nalData, nalData + nalLen);
    onNalUnit(std::move(data));
}

void RtpDepacketizer::flushAccessUnit()
{
    if (auBuffer_.empty()) return;

    // 将缓冲中所有 NAL Unit 合并为一个 Access Unit 输出
    // 每个 NAL 已自带起始码 (00 00 00 01)
    for (auto& nal : auBuffer_) {
        if (onNalUnit) {
            onNalUnit(std::move(nal));
        }
    }
    auBuffer_.clear();
}

} // namespace ms

