#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "core/StreamSession.h"
#include "core/Types.h"
#include "mediastream.h"

#ifdef MS_ENABLE_WEBRTC
#include "webrtc/WebRtcSource.h"
#endif
#include "rtsp/RtspSource.h"

#include <cstring>

namespace ms {

StreamSession::StreamSession(ms_source_type type)
{
    source_ = createSource(type);
    if (source_) {
        // 绑定内部回调
        source_->onFrame = [this](const YuvFrame& frame) {
            onFrameReady(frame);
        };
        source_->onState = [this](StreamState st, const std::string& msg) {
            onStateChanged(st, msg);
        };
    }
}

StreamSession::~StreamSession()
{
    stop();
}

int StreamSession::setUrl(const char* url)
{
    if (!url) return -1;
    std::lock_guard<std::mutex> lock(mutex_);
    url_ = url;
    return 0;
}

int StreamSession::setOption(const char* key, const char* value)
{
    if (!key || !value) return -1;
    std::lock_guard<std::mutex> lock(mutex_);
    options_[key] = value;
    return 0;
}

void StreamSession::setFrameCallback(ms_frame_cb cb, void* userdata)
{
    std::lock_guard<std::mutex> lock(mutex_);
    frameCb_ = cb;
    frameUd_ = userdata;
}

void StreamSession::setStateCallback(ms_state_cb cb, void* userdata)
{
    std::lock_guard<std::mutex> lock(mutex_);
    stateCb_ = cb;
    stateUd_ = userdata;
}

int StreamSession::start()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!source_) return -1;
    if (url_.empty()) return -2;
    if (state_ == MS_STATE_PLAYING || state_ == MS_STATE_CONNECTING) return -3;

    state_ = MS_STATE_CONNECTING;

    int ret = source_->open(url_, options_);
    if (ret != 0) {
        state_ = MS_STATE_ERROR;
        return ret;
    }

    ret = source_->start();
    if (ret != 0) {
        state_ = MS_STATE_ERROR;
        return ret;
    }

    return 0;
}

int StreamSession::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!source_) return -1;
    if (state_ == MS_STATE_STOPPED) return 0;

    source_->stop();
    state_ = MS_STATE_STOPPED;
    return 0;
}

ms_state StreamSession::state() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::unique_ptr<IStreamSource> StreamSession::createSource(ms_source_type type)
{
    switch (type) {
    case MS_SOURCE_RTSP:
        return std::make_unique<RtspSource>();

#ifdef MS_ENABLE_WEBRTC
    case MS_SOURCE_WEBRTC:
        return std::make_unique<WebRtcSource>();
#endif

    default:
        return nullptr;
    }
}

void StreamSession::onFrameReady(const YuvFrame& frame)
{
    ms_frame_cb  cb = nullptr;
    void*        ud = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cb = frameCb_;
        ud = frameUd_;
    }

    if (cb) {
        ms_frame f{};
        f.y        = frame.y;
        f.u        = frame.u;
        f.v        = frame.v;
        f.width    = frame.width;
        f.height   = frame.height;
        f.y_stride = frame.yStride;
        f.uv_stride = frame.uvStride;
        f.pts_ms   = frame.ptsMs;

        cb(static_cast<ms_session_t>(this), &f, ud);
    }
}

void StreamSession::onStateChanged(StreamState st, const std::string& msg)
{
    ms_state_cb  cb = nullptr;
    void*        ud = nullptr;
    ms_state     newState;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        switch (st) {
        case StreamState::Stopped:    newState = MS_STATE_STOPPED;    break;
        case StreamState::Connecting: newState = MS_STATE_CONNECTING; break;
        case StreamState::Playing:    newState = MS_STATE_PLAYING;    break;
        case StreamState::Error:      newState = MS_STATE_ERROR;      break;
        default:                      newState = MS_STATE_ERROR;      break;
        }
        state_ = newState;
        cb = stateCb_;
        ud = stateUd_;
    }

    if (cb) {
        cb(static_cast<ms_session_t>(this), newState,
           msg.empty() ? nullptr : msg.c_str(), ud);
    }
}

} // namespace ms

