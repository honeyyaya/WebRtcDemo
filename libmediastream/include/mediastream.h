#ifndef MEDIASTREAM_H
#define MEDIASTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef _WIN32
  #ifdef MEDIASTREAM_EXPORTS
    #define MS_API __declspec(dllexport)
  #else
    #define MS_API __declspec(dllimport)
  #endif
#else
  #define MS_API __attribute__((visibility("default")))
#endif

/* ========== 类型定义 ========== */

typedef void* ms_session_t;

typedef enum {
    MS_SOURCE_RTSP   = 0,
    MS_SOURCE_WEBRTC = 1,
} ms_source_type;

typedef enum {
    MS_STATE_STOPPED    = 0,
    MS_STATE_CONNECTING = 1,
    MS_STATE_PLAYING    = 2,
    MS_STATE_ERROR      = 3,
} ms_state;

typedef struct {
    const uint8_t* y;
    const uint8_t* u;
    const uint8_t* v;
    int width;
    int height;
    int y_stride;
    int uv_stride;
    int64_t pts_ms;
} ms_frame;

/* ========== 回调定义 ========== */

typedef void (*ms_frame_cb)(ms_session_t session,
                            const ms_frame* frame,
                            void* userdata);

typedef void (*ms_state_cb)(ms_session_t session,
                            ms_state state,
                            const char* msg,
                            void* userdata);

/* ========== 生命周期 ========== */

MS_API ms_session_t ms_create(ms_source_type type);
MS_API void         ms_destroy(ms_session_t session);

/* ========== 配置 ========== */

MS_API int  ms_set_url(ms_session_t session, const char* url);
MS_API int  ms_set_option(ms_session_t session,
                          const char* key,
                          const char* value);

/* ========== 回调注册 ========== */

MS_API void ms_set_frame_callback(ms_session_t session,
                                  ms_frame_cb cb,
                                  void* userdata);
MS_API void ms_set_state_callback(ms_session_t session,
                                  ms_state_cb cb,
                                  void* userdata);

/* ========== 控制 ========== */

MS_API int  ms_start(ms_session_t session);
MS_API int  ms_stop(ms_session_t session);

/* ========== 辅助 ========== */

MS_API const char* ms_version(void);
MS_API const char* ms_state_name(ms_state state);

#ifdef __cplusplus
}
#endif

#endif /* MEDIASTREAM_H */

