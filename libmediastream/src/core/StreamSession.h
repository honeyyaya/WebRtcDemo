#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "IStreamSource.h"
#include "mediastream.h"
#include <memory>
#include <mutex>

namespace ms {

/// 会话管理
/// 封装一个 IStreamSource，管理配置、回调和生命周期
class StreamSession {
public:
    explicit StreamSession(ms_source_type type);
    ~StreamSession();

    // 禁止拷贝
    StreamSession(const StreamSession&) = delete;
    StreamSession& operator=(const StreamSession&) = delete;

    int  setUrl(const char* url);
    int  setOption(const char* key, const char* value);
    void setFrameCallback(ms_frame_cb cb, void* userdata);
    void setStateCallback(ms_state_cb cb, void* userdata);

    int  start();
    int  stop();
    ms_state state() const;

private:
    /// 工厂方法：根据 source type 创建对应的 IStreamSource
    static std::unique_ptr<IStreamSource> createSource(ms_source_type type);

    /// 内部帧回调 —— 由 IStreamSource 触发
    void onFrameReady(const YuvFrame& frame);

    /// 内部状态回调 —— 由 IStreamSource 触发
    void onStateChanged(StreamState st, const std::string& msg);

private:
    std::unique_ptr<IStreamSource> source_;
    std::string url_;
    Options     options_;

    ms_frame_cb  frameCb_  = nullptr;
    void*        frameUd_  = nullptr;
    ms_state_cb  stateCb_  = nullptr;
    void*        stateUd_  = nullptr;

    ms_state         state_ = MS_STATE_STOPPED;
    mutable std::mutex mutex_;
};

} // namespace ms

