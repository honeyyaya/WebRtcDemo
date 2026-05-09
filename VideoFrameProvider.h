#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/// @file VideoFrameProvider.h
/// @brief QML 视频帧提供者
///
/// 从 libmediastream 的 ms_frame_cb 回调中接收 YUV 帧，
/// 转换为 QImage 供 QML Image 组件显示。
/// 同时封装 ms_session_t 的创建/启动/停止逻辑，
/// 作为 QML 可用的 C++ 后端对象。

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <QMutex>
#include <QString>

#include "mediastream.h"
// submodel 测试

/// 异步图片提供者 —— 供 QML Image 组件通过 "image://video/frame" 获取最新帧
class VideoImageProvider : public QQuickImageProvider
{
public:
    VideoImageProvider();

    QImage requestImage(const QString& id, QSize* size,
                        const QSize& requestedSize) override;

    /// 更新当前帧（线程安全）
    void updateFrame(const QImage& frame);

private:
    QMutex mutex_;
    QImage currentFrame_;
};

/// QML 可用的视频控制器
/// 负责管理 ms_session_t 生命周期，并在收到帧时通知 QML 刷新
class VideoFrameProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    explicit VideoFrameProvider(QObject* parent = nullptr);
    ~VideoFrameProvider() override;

    /// 设置关联的图片提供者（由 main.cpp 在注册时设置）
    void setImageProvider(VideoImageProvider* provider);

    QString statusText() const;
    bool connected() const;

    /// 供 QML 调用：连接并开始播放
    Q_INVOKABLE void startPlay(const QString& signalingUrl, const QString& room);

    /// 供 QML 调用：停止播放
    Q_INVOKABLE void stopPlay();

signals:
    /// 有新帧可用，QML 端收到后刷新 Image source
    void frameUpdated();

    void statusChanged();
    void connectedChanged();

private:
    /// ms_frame_cb 静态回调
    static void onFrame(ms_session_t session, const ms_frame* frame, void* userdata);

    /// ms_state_cb 静态回调
    static void onState(ms_session_t session, ms_state state,
                        const char* msg, void* userdata);

    /// 将 YUV420P 转换为 QImage (RGB32)
    static QImage yuvToQImage(const ms_frame* frame);

    ms_session_t       session_  = nullptr;
    VideoImageProvider* provider_ = nullptr;
    QString            statusText_ = QStringLiteral("未连接");
    bool               connected_  = false;
    int                frameCounter_ = 0;
};

