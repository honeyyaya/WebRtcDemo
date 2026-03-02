import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 960
    height: 640
    title: qsTr("WebRTC 接收端 - Phase 2.1")
    visible: true
    color: "#1a1a2e"

    // 帧计数器，用于刷新 Image source
    property int frameCount: 0

    Connections {
        target: videoProvider
        function onFrameUpdated() {
            frameCount++
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ==================== 视频显示区域 ====================
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000000"

            Image {
                id: videoImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                cache: false
                source: "image://video/frame?" + frameCount
            }

            // 无视频时的占位提示
            Label {
                anchors.centerIn: parent
                text: "等待视频..."
                color: "#666666"
                font.pixelSize: 24
                visible: frameCount === 0
            }
        }

        // ==================== 控制栏 ====================
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: "#2d2d2d"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // 状态标签
                Label {
                    id: statusLabel
                    text: videoProvider.statusText
                    color: videoProvider.connected ? "#4CAF50" : "#FFC107"
                    font.pixelSize: 13
                    Layout.preferredWidth: 180
                }

                Item { Layout.fillWidth: true }

                // 信令服务器地址
                Label {
                    text: "信令服务器:"
                    color: "#cccccc"
                    font.pixelSize: 13
                }
                TextField {
                    id: urlField
                    text: "ws://192.168.1.100:3000/ws"
                    Layout.preferredWidth: 280
                    font.pixelSize: 13
                    selectByMouse: true
                    color: "#ffffff"
                    background: Rectangle {
                        color: "#3d3d3d"
                        border.color: "#555555"
                        radius: 4
                    }
                    onAccepted: connectBtn.clicked()
                }

                // 房间名
                Label {
                    text: "房间:"
                    color: "#cccccc"
                    font.pixelSize: 13
                }
                TextField {
                    id: roomField
                    text: "video-room"
                    Layout.preferredWidth: 120
                    font.pixelSize: 13
                    selectByMouse: true
                    color: "#ffffff"
                    background: Rectangle {
                        color: "#3d3d3d"
                        border.color: "#555555"
                        radius: 4
                    }
                }

                // 连接按钮
                Button {
                    id: connectBtn
                    text: videoProvider.connected ? "断开" : "连接"
                    onClicked: {
                        if (videoProvider.connected) {
                            videoProvider.stopPlay()
                            frameCount = 0
                        } else {
                            videoProvider.startPlay(urlField.text, roomField.text)
                        }
                    }
                }
            }
        }
    }
}
