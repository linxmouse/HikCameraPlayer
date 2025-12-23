import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
// 导入模块以使用 AppController
import HikRealPlayer

ApplicationWindow {
    id: window
    width: 800
    height: 450
    visible: true
    title: "CCTV Player"

    // 创建c++对象实例
    AppController {
        id: controller
    }

    // 独立的视频预览窗口 (无边框，用于嵌入效果)
    Window {
        id: videoWindow
        width: videoPlaceholder.width
        height: videoPlaceholder.height
        flags: Qt.FramelessWindowHint | Qt.Window // 无边框
        color: "transparent"
        visible: controller.isPlaying

        onClosing: {
            controller.stopPlay()
        }

        // 实时同步位置
        Timer {
            interval: 16
            running: videoWindow.visible
            repeat: true
            onTriggered: {
                var globalPos = videoPlaceholder.mapToGlobal(0, 0)
                videoWindow.x = globalPos.x
                videoWindow.y = globalPos.y
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6

        // 登录栏
        RowLayout {
            spacing: 6
            Label {
                text: "IP:"
            }
            TextField {
                id: ipField
                text: "192.168.0.65"
                implicitWidth: 100
            }
            Label {
                text: "端口:"
            }
            TextField {
                id: portField
                text: "8000"
                implicitWidth: 60
            }
            Label {
                text: "用户名:"
            }
            TextField {
                id: userField
                text: "admin"
                implicitWidth: 80
            }
            Label {
                text: "密码:"
            }
            TextField {
                id: passField
                text: "ty123456"
                implicitWidth: 100
                echoMode: TextInput.Password
            }
            Label {
                text: "通道:"
            }
            TextField {
                id: channelField
                text: controller.channel.toString()
                implicitWidth: 40
                onTextChanged: controller.channel = parseInt(text) || 1
            }
            Button {
                id: loginBtn
                text: controller.isLoggedIn ? "登出" : "登录"
                onClicked: {
                    if (controller.isLoggedIn) {
                        controller.logout()
                    } else {
                        controller.login(ipField.text,
                                         parseInt(portField.text),
                                         userField.text, passField.text)
                    }
                }
            }
        }

        // 控制按钮栏
        RowLayout {
            spacing: 6
            Button {
                id: playBtn
                text: controller.isPlaying ? "停止" : "播放"
                enabled: controller.isLoggedIn
                onClicked: {
                    if (controller.isPlaying) {
                        controller.stopPlay()
                    } else {
                        controller.startPlay(videoWindow)
                    }
                }
            }
            Button {
                text: "拍照"
                enabled: controller.isPlaying
                onClicked: controller.capture()
            }
        }

        // 视频显示区域 (占位符)
        Rectangle {
            id: videoPlaceholder
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "black"
            Label {
                anchors.centerIn: parent
                color: "gray"
                text: controller.isPlaying ? "" : "视频预览区域"
            }
        }

        // 状态栏
        Label {
            id: statusLabel
            text: controller.statusMessage
            color: "darkgreen"
            font.bold: true
        }
    }
}
