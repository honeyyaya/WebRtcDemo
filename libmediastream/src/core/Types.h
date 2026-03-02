#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <cstdint>
#include <string>
#include <vector>

namespace ms {

/// 内部 YUV 帧结构，与对外 ms_frame 对应
struct YuvFrame {
    const uint8_t* y = nullptr;
    const uint8_t* u = nullptr;
    const uint8_t* v = nullptr;
    int width    = 0;
    int height   = 0;
    int yStride  = 0;
    int uvStride = 0;
    int64_t ptsMs = 0;

    /// 帧数据的持有缓冲区（可选，用于内部拷贝帧数据时）
    std::vector<uint8_t> buffer;
};

/// 流状态枚举（内部使用，与对外 ms_state 对应）
enum class StreamState {
    Stopped    = 0,
    Connecting = 1,
    Playing    = 2,
    Error      = 3,
};

/// RTP 包结构（WebRTC 模块内部使用）
struct RtpPacket {
    uint16_t seq       = 0;
    uint32_t timestamp = 0;
    bool     marker    = false;
    uint8_t  payloadType = 0;
    uint32_t ssrc      = 0;
    std::vector<uint8_t> payload;
    int64_t  arrivalTimeMs = 0;  // 本地到达时间
};

} // namespace ms

