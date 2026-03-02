/// @file api.cpp
/// @brief C API 桥接实现 —— 将所有 ms_* 函数转发到 StreamSession

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "mediastream.h"
#include "core/StreamSession.h"

#include <cstring>
#include <new>

// 版本号
static const char* kVersion = "1.0.0";

// 状态名称
static const char* kStateNames[] = {
    "STOPPED",
    "CONNECTING",
    "PLAYING",
    "ERROR",
};

/// 将 ms_session_t (void*) 转换为 StreamSession*
static inline ms::StreamSession* toSession(ms_session_t s)
{
    return static_cast<ms::StreamSession*>(s);
}

/* ========== 生命周期 ========== */

ms_session_t ms_create(ms_source_type type)
{
    try {
        auto* session = new ms::StreamSession(type);
        return static_cast<ms_session_t>(session);
    } catch (...) {
        return nullptr;
    }
}

void ms_destroy(ms_session_t session)
{
    if (!session) return;
    try {
        auto* s = toSession(session);
        s->stop();
        delete s;
    } catch (...) {
        // 忽略析构异常
    }
}

/* ========== 配置 ========== */

int ms_set_url(ms_session_t session, const char* url)
{
    if (!session || !url) return -1;
    try {
        return toSession(session)->setUrl(url);
    } catch (...) {
        return -1;
    }
}

int ms_set_option(ms_session_t session, const char* key, const char* value)
{
    if (!session || !key || !value) return -1;
    try {
        return toSession(session)->setOption(key, value);
    } catch (...) {
        return -1;
    }
}

/* ========== 回调注册 ========== */

void ms_set_frame_callback(ms_session_t session, ms_frame_cb cb, void* userdata)
{
    if (!session) return;
    try {
        toSession(session)->setFrameCallback(cb, userdata);
    } catch (...) {}
}

void ms_set_state_callback(ms_session_t session, ms_state_cb cb, void* userdata)
{
    if (!session) return;
    try {
        toSession(session)->setStateCallback(cb, userdata);
    } catch (...) {}
}

/* ========== 控制 ========== */

int ms_start(ms_session_t session)
{
    if (!session) return -1;
    try {
        return toSession(session)->start();
    } catch (...) {
        return -1;
    }
}

int ms_stop(ms_session_t session)
{
    if (!session) return -1;
    try {
        return toSession(session)->stop();
    } catch (...) {
        return -1;
    }
}

/* ========== 辅助 ========== */

const char* ms_version(void)
{
    return kVersion;
}

const char* ms_state_name(ms_state state)
{
    if (state >= 0 && state <= MS_STATE_ERROR) {
        return kStateNames[state];
    }
    return "UNKNOWN";
}

