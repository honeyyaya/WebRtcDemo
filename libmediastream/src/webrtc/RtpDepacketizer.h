#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file RtpDepacketizer.h
/// @brief H.264 RTP 解包器 [Phase 2.1 完整实现]
///
/// 解析 RTP 头部，提取 seq/timestamp/marker/payload，
/// 处理三种 NAL 封装模式：
///   - Single NAL Unit (type 1-23)：一个 RTP 包 = 一个 NAL
///   - STAP-A (type 24)：一个 RTP 包 = 多个小 NAL（如 SPS+PPS）
///   - FU-A (type 28)：一个大 NAL 分片到多个 RTP 包
///
/// 帧完整时（marker=1）通过 onNalUnit 回调输出完整 NAL Unit

#include <cstdint>
#include <cstddef>
#include <vector>
#include <functional>

namespace ms {

class RtpDepacketizer {
public:
    RtpDepacketizer();
    ~RtpDepacketizer();

    /// 输入一个完整的 RTP 包（含 RTP 头）
    /// 内部解析并组帧，完成后触发 onNalUnit 回调
    void feed(const uint8_t* data, size_t len);

    /// 重置内部状态
    void reset();

    /// 完整 NAL Unit 输出回调
    /// 参数为包含 NAL 起始码 (00 00 00 01) 的完整数据
    std::function<void(std::vector<uint8_t> nalUnit)> onNalUnit;

private:
    /// 解析 RTP 头部，返回 payload 起始偏移和长度
    struct RtpHeader {
        uint16_t seq       = 0;
        uint32_t timestamp = 0;
        bool     marker    = false;
        uint8_t  payloadType = 0;
        uint32_t ssrc      = 0;
        int      headerLen = 0;  // RTP 头部长度（含 CSRC 和扩展）
    };

    bool parseRtpHeader(const uint8_t* data, size_t len, RtpHeader& header);

    /// 处理单个 NAL Unit (type 1-23)
    void parseSingleNal(const uint8_t* payload, size_t len,
                        uint32_t timestamp, bool marker);

    /// 处理 FU-A 分片 (type 28)
    void parseFuA(const uint8_t* payload, size_t len,
                  uint32_t timestamp, bool marker);

    /// 处理 STAP-A 聚合 (type 24)
    void parseStapA(const uint8_t* payload, size_t len,
                    uint32_t timestamp, bool marker);

    /// 输出一个 NAL Unit（自动添加起始码 00 00 00 01）
    void emitNalUnit(const uint8_t* nalData, size_t nalLen);

    /// 当一个 Access Unit（帧）完成时，刷新缓冲并输出
    void flushAccessUnit();

private:
    // FU-A 分片重组缓冲
    std::vector<uint8_t> fuBuffer_;
    bool                 fuActive_ = false;
    uint32_t             fuTimestamp_ = 0;

    // Access Unit 缓冲（同一时间戳的 NAL 集合）
    std::vector<std::vector<uint8_t>> auBuffer_;
    uint32_t                          currentTimestamp_ = 0;
    bool                              hasTimestamp_ = false;

    // 序列号跟踪
    uint16_t lastSeq_    = 0;
    bool     hasLastSeq_ = false;
};

} // namespace ms

