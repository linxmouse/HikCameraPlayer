import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Controls.Material 6.0
import QtQuick.Layouts 6.0
import QtMultimedia
import work.dbugs.hikcamera 1.0

ApplicationWindow {
    id: window
    minimumWidth: 950
    minimumHeight: 600
    visible: true
    title: "Hikvision Player (Material Theme)"

    // 设置默认主题
    Material.theme: Material.System
    Material.accent: Material.Blue

    AppController {
        id: controller
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5

        // 顶部工具栏 (更紧凑)
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 30

            Label {
                text: "HikPlayer"
                font.pixelSize: 14
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: 8
                Label {
                    text: "主题:"
                    font.pixelSize: 12
                }
                Switch {
                    id: themeSwitch
                    scale: 0.8
                    checked: window.Material.theme === Material.Light
                    text: checked ? "浅色" : "深色"
                    onToggled: window.Material.theme = checked ? Material.Light : Material.Dark
                }
                ComboBox {
                    model: ["Blue", "Orange", "Teal", "Purple"]
                    scale: 0.8
                    onActivated: index => {
                        const colors = [Material.Blue, Material.Orange, Material.Teal, Material.Purple];
                        window.Material.accent = colors[index];
                    }
                }
            }
        }

        // 综合控制面板
        Pane {
            Layout.fillWidth: true
            Material.elevation: 1
            padding: 5

            ColumnLayout {
                width: parent.width
                spacing: 2

                // 第一行：登录与基础操作
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    TextField {
                        id: ipField
                        placeholderText: "IP"
                        text: "192.168.0.65"
                        Layout.preferredWidth: 110
                        font.pixelSize: 12
                        topPadding: 6
                        bottomPadding: 6
                    }
                    TextField {
                        id: portField
                        placeholderText: "端口"
                        text: "8000"
                        Layout.preferredWidth: 60
                        font.pixelSize: 12
                        topPadding: 6
                        bottomPadding: 6
                    }
                    TextField {
                        id: userField
                        placeholderText: "用户"
                        text: "admin"
                        Layout.preferredWidth: 80
                        font.pixelSize: 12
                        topPadding: 6
                        bottomPadding: 6
                    }
                    TextField {
                        id: passField
                        placeholderText: "密码"
                        text: "ty123456"
                        echoMode: TextInput.Password
                        Layout.preferredWidth: 100
                        font.pixelSize: 12
                        topPadding: 6
                        bottomPadding: 6
                    }

                    Label {
                        text: "通道:"
                        font.pixelSize: 12
                    }
                    SpinBox {
                        id: channelSpin
                        from: 1
                        to: 64
                        value: controller.channel
                        scale: 0.8
                        onValueChanged: controller.channel = value
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: controller.isLoggedIn ? "登出" : "登录"
                        font.pixelSize: 12
                        onClicked: controller.isLoggedIn ? controller.logout() : controller.login(ipField.text, parseInt(portField.text), userField.text, passField.text)
                    }
                    Button {
                        text: controller.isPlaying ? "停止" : "播放"
                        enabled: controller.isLoggedIn
                        font.pixelSize: 12
                        onClicked: controller.isPlaying ? controller.stopPlay() : controller.startPlay()
                    }
                    Button {
                        text: "拍照"
                        enabled: controller.isPlaying
                        font.pixelSize: 12
                        onClicked: controller.capture()
                    }
                }

                // 第二行：PTZ 控制 (仅登录后显示)
                RowLayout {
                    Layout.fillWidth: true
                    visible: controller.isLoggedIn
                    spacing: 10

                    Label {
                        text: "云台:"
                        font.bold: true
                        font.pixelSize: 12
                    }

                    RowLayout {
                        spacing: 2
                        Label {
                            text: "缩放:"
                            font.pixelSize: 12
                        }
                        Button {
                            text: "拉近+"
                            scale: 0.8
                            enabled: controller.isPlaying
                            onPressed: controller.ptzControl(AppController.ZoomIn, false, ptzSpeedSlider.value)
                            onReleased: controller.ptzControl(AppController.ZoomIn, true, ptzSpeedSlider.value)
                        }
                        Button {
                            text: "拉远-"
                            scale: 0.8
                            enabled: controller.isPlaying
                            onPressed: controller.ptzControl(AppController.ZoomOut, false, ptzSpeedSlider.value)
                            onReleased: controller.ptzControl(AppController.ZoomOut, true, ptzSpeedSlider.value)
                        }
                    }

                    RowLayout {
                        spacing: 2
                        Label {
                            text: "聚焦:"
                            font.pixelSize: 12
                        }
                        Button {
                            text: "近"
                            scale: 0.8
                            enabled: controller.isPlaying
                            onPressed: controller.ptzControl(AppController.FocusNear, false, ptzSpeedSlider.value)
                            onReleased: controller.ptzControl(AppController.FocusNear, true, ptzSpeedSlider.value)
                        }
                        Button {
                            text: "远"
                            scale: 0.8
                            enabled: controller.isPlaying
                            onPressed: controller.ptzControl(AppController.FocusFar, false, ptzSpeedSlider.value)
                            onReleased: controller.ptzControl(AppController.FocusFar, true, ptzSpeedSlider.value)
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 2
                        Label {
                            text: "速度:"
                            font.pixelSize: 12
                        }
                        Slider {
                            id: ptzSpeedSlider
                            from: 1
                            to: 7
                            value: 4
                            stepSize: 1
                            Layout.preferredWidth: 100
                        }
                        Label {
                            text: ptzSpeedSlider.value.toString()
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        // 视频区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "black"
            radius: 4
            clip: true

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                visible: controller.isPlaying
                Component.onCompleted: controller.videoSink = videoOutput.videoSink
            }

            Label {
                anchors.centerIn: parent
                text: "无信号"
                color: "gray"
                visible: !controller.isPlaying
            }
        }

        // 状态栏
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: controller.statusMessage
                font.italic: true
                Layout.fillWidth: true
            }
            Label {
                text: "© 2025 linxmouse | Qt 6.10 | Material Style"
                font.pixelSize: 10
                opacity: 0.6
            }
        }
    }
}
