#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "VideoFrameProvider.h"
#include "mediastream.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建视频图片提供者（QML 通过 "image://video/frame" 访问）
    auto* imageProvider = new VideoImageProvider();
    engine.addImageProvider(QStringLiteral("video"), imageProvider);

    // 创建视频控制器并关联图片提供者
    auto* videoProvider = new VideoFrameProvider(&app);
    videoProvider->setImageProvider(imageProvider);

    // 注册到 QML 上下文
    engine.rootContext()->setContextProperty("videoProvider", videoProvider);

    // 加载 QML
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("WebRtcDemo", "Main");

    return app.exec();
}
