/// @file test/main.cpp
/// @brief libmediastream 独立测试程序
///
/// 演示如何使用 C API 进行 WebRTC 取流
/// 编译: cmake --build . --target mediastream_test

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "mediastream.h"

#include <cstdio>
#include <cstdlib>
#include <csignal>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

// 全局标志，用于控制主循环退出
static volatile bool g_running = true;

static void signalHandler(int /*sig*/)
{
    g_running = false;
}

// 帧回调
static void onFrame(ms_session_t /*session*/, const ms_frame* frame, void* /*userdata*/)
{
    static int count = 0;
    count++;
    if (count % 30 == 0) {  // 每 30 帧打印一次
        printf("[TEST] 收到帧 #%d: %dx%d  pts=%lld\n",
               count, frame->width, frame->height,
               (long long)frame->pts_ms);
    }
}

// 状态回调
static void onState(ms_session_t /*session*/, ms_state state,
                    const char* msg, void* /*userdata*/)
{
    printf("[TEST] 状态变化: %s", ms_state_name(state));
    if (msg && msg[0]) {
        printf(" - %s", msg);
    }
    printf("\n");
}

int main(int argc, char* argv[])
{
    printf("========================================\n");
    printf("  libmediastream 测试程序 v%s\n", ms_version());
    printf("========================================\n\n");

    // 默认参数
    const char* url  = "ws://192.168.1.100:3000/ws";
    const char* room = "video-room";

    if (argc >= 2) url  = argv[1];
    if (argc >= 3) room = argv[2];

    printf("信令服务器: %s\n", url);
    printf("房间名称:   %s\n\n", room);

    // 注册信号处理
    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);

    // 1. 创建 WebRTC 会话
    ms_session_t session = ms_create(MS_SOURCE_WEBRTC);
    if (!session) {
        fprintf(stderr, "[ERROR] 创建会话失败\n");
        return 1;
    }

    // 2. 配置
    ms_set_url(session, url);
    ms_set_option(session, "room", room);

    // 3. 注册回调
    ms_set_frame_callback(session, onFrame, nullptr);
    ms_set_state_callback(session, onState, nullptr);

    // 4. 启动
    int ret = ms_start(session);
    if (ret != 0) {
        fprintf(stderr, "[ERROR] 启动失败, ret=%d\n", ret);
        ms_destroy(session);
        return 1;
    }

    printf("会话已启动，等待视频... (按 Ctrl+C 退出)\n\n");

    // 5. 主循环等待
    while (g_running) {
        SLEEP_MS(100);
    }

    printf("\n正在停止...\n");

    // 6. 停止并销毁
    ms_stop(session);
    ms_destroy(session);

    printf("已退出\n");
    return 0;
}

