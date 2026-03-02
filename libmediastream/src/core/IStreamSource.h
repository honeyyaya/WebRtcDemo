#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "Types.h"
#include <functional>
#include <string>
#include <unordered_map>

namespace ms {

using Options = std::unordered_map<std::string, std::string>;

/// 取流源抽象接口
/// RtspSource 和 WebRtcSource 均实现此接口
class IStreamSource {
public:
    virtual ~IStreamSource() = default;

    /// 打开流源，传入 URL 和选项
    /// @return 0 成功，非 0 失败
    virtual int  open(const std::string& url, const Options& opts) = 0;

    /// 开始取流
    /// @return 0 成功，非 0 失败
    virtual int  start() = 0;

    /// 停止取流
    virtual void stop()  = 0;

    /// 解码帧回调 —— 由 StreamSession 设置
    std::function<void(const YuvFrame&)>                  onFrame;

    /// 状态变化回调 —— 由 StreamSession 设置
    std::function<void(StreamState, const std::string&)>  onState;
};

} // namespace ms

