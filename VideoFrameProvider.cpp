/// @file VideoFrameProvider.cpp
/// @brief QML 视频帧提供者实现

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "VideoFrameProvider.h"

#include <QDebug>
#include <algorithm>
#include <cstring>

// =============================================================================
// VideoImageProvider
// =============================================================================

VideoImageProvider::VideoImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage VideoImageProvider::requestImage(const QString& /*id*/, QSize* size,
                                         const QSize& requestedSize)
{
    QMutexLocker lock(&mutex_);

    if (currentFrame_.isNull()) {
        // 返回占位黑色图像
        QImage placeholder(requestedSize.isValid() ? requestedSize : QSize(640, 480),
                           QImage::Format_RGB32);
        placeholder.fill(Qt::black);
        if (size) *size = placeholder.size();
        return placeholder;
    }

    QImage result = currentFrame_;
    if (requestedSize.isValid() && requestedSize != result.size()) {
        result = result.scaled(requestedSize, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    }
    if (size) *size = result.size();
    return result;
}

void VideoImageProvider::updateFrame(const QImage& frame)
{
    QMutexLocker lock(&mutex_);
    currentFrame_ = frame;
}

// =============================================================================
// VideoFrameProvider
// =============================================================================

VideoFrameProvider::VideoFrameProvider(QObject* parent)
    : QObject(parent)
{
}

VideoFrameProvider::~VideoFrameProvider()
{
    stopPlay();
}

void VideoFrameProvider::setImageProvider(VideoImageProvider* provider)
{
    provider_ = provider;
}

QString VideoFrameProvider::statusText() const
{
    return statusText_;
}

bool VideoFrameProvider::connected() const
{
    return connected_;
}

void VideoFrameProvider::startPlay(const QString& signalingUrl, const QString& room)
{
    // 如果已有会话，先停止
    stopPlay();

    // 创建 WebRTC 会话
    session_ = ms_create(MS_SOURCE_WEBRTC);
    if (!session_) {
        statusText_ = QStringLiteral("创建会话失败");
        emit statusChanged();
        return;
    }

    // 设置信令 URL
    ms_set_url(session_, signalingUrl.toUtf8().constData());

    // 设置房间名
    if (!room.isEmpty()) {
        ms_set_option(session_, "room", room.toUtf8().constData());
    }

    // 注册回调
    ms_set_frame_callback(session_, &VideoFrameProvider::onFrame, this);
    ms_set_state_callback(session_, &VideoFrameProvider::onState, this);

    // 启动
    int ret = ms_start(session_);
    if (ret != 0) {
        statusText_ = QStringLiteral("启动失败 (err=%1)").arg(ret);
        emit statusChanged();
        ms_destroy(session_);
        session_ = nullptr;
        return;
    }

    statusText_ = QStringLiteral("正在连接...");
    emit statusChanged();
}

void VideoFrameProvider::stopPlay()
{
    if (session_) {
        ms_stop(session_);
        ms_destroy(session_);
        session_ = nullptr;
    }

    connected_ = false;
    statusText_ = QStringLiteral("已停止");
    emit statusChanged();
    emit connectedChanged();
}

void VideoFrameProvider::onFrame(ms_session_t /*session*/,
                                  const ms_frame* frame,
                                  void* userdata)
{
    auto* self = static_cast<VideoFrameProvider*>(userdata);
    if (!self || !self->provider_ || !frame) return;

    // YUV → QImage 转换
    QImage img = yuvToQImage(frame);
    if (img.isNull()) return;

    // 更新图片提供者
    self->provider_->updateFrame(img);

    // 通知 QML 刷新（跨线程安全，信号槽自动排队）
    emit self->frameUpdated();
}

void VideoFrameProvider::onState(ms_session_t /*session*/,
                                  ms_state state,
                                  const char* msg,
                                  void* userdata)
{
    auto* self = static_cast<VideoFrameProvider*>(userdata);
    if (!self) return;

    QString stateStr = QString::fromUtf8(ms_state_name(state));
    QString message  = msg ? QString::fromUtf8(msg) : QString();

    switch (state) {
    case MS_STATE_CONNECTING:
        self->statusText_ = QStringLiteral("正在连接: %1").arg(message);
        self->connected_ = false;
        break;
    case MS_STATE_PLAYING:
        self->statusText_ = QStringLiteral("正在播放");
        self->connected_ = true;
        break;
    case MS_STATE_ERROR:
        self->statusText_ = QStringLiteral("错误: %1").arg(message);
        self->connected_ = false;
        break;
    case MS_STATE_STOPPED:
        self->statusText_ = QStringLiteral("已停止");
        self->connected_ = false;
        break;
    }

    emit self->statusChanged();
    emit self->connectedChanged();
}

QImage VideoFrameProvider::yuvToQImage(const ms_frame* frame)
{
    if (!frame || !frame->y || frame->width <= 0 || frame->height <= 0) {
        return QImage();
    }

    int w = frame->width;
    int h = frame->height;

    QImage img(w, h, QImage::Format_RGB32);

    // YUV420P → RGB32 简单转换
    // 注意：这是 CPU 转换，后续可优化为 OpenGL shader
    for (int y = 0; y < h; ++y) {
        const uint8_t* yLine  = frame->y + y * frame->y_stride;
        const uint8_t* uLine  = frame->u + (y / 2) * frame->uv_stride;
        const uint8_t* vLine  = frame->v + (y / 2) * frame->uv_stride;
        QRgb* rgbLine = reinterpret_cast<QRgb*>(img.scanLine(y));

        for (int x = 0; x < w; ++x) {
            int Y = yLine[x];
            int U = uLine[x / 2] - 128;
            int V = vLine[x / 2] - 128;

            int R = Y + (int)(1.402 * V);
            int G = Y - (int)(0.344136 * U) - (int)(0.714136 * V);
            int B = Y + (int)(1.772 * U);

            R = std::clamp(R, 0, 255);
            G = std::clamp(G, 0, 255);
            B = std::clamp(B, 0, 255);

            rgbLine[x] = qRgb(R, G, B);
        }
    }

    return img;
}

