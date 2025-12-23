import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Controls.Material 6.0
import QtQuick.Layouts 6.0
import QtMultimedia
import HikCameraPlayer 1.0

ApplicationWindow {
    id: window
    width: 900
    height: 600
    visible: true
    title: "Hikvision Player (Material Theme)"

    // 设置默认主题
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    AppController {
        id: controller
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 顶部工具栏
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "CameraPlayer By Linxmouse"
                font.pixelSize: 16
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            // 内置主题切换
            RowLayout {
                spacing: 10
                Label {
                    text: "主题:"
                }
                Switch {
                    id: themeSwitch
                    text: checked ? "浅色" : "深色"
                    checked: false
                    onCheckedChanged: {
                        window.Material.theme = checked ? Material.Light : Material.Dark;
                    }
                }

                ComboBox {
                    model: ["Blue", "Red", "Green", "Amber", "Purple"]
                    onActivated: index => {
                        const colors = [Material.Blue, Material.Red, Material.Green, Material.Amber, Material.Purple];
                        window.Material.accent = colors[index];
                    }
                }
            }
        }

        // 登录面板
        Pane {
            Layout.fillWidth: true
            Material.elevation: 2

            RowLayout {
                anchors.fill: parent
                spacing: 10

                TextField {
                    id: ipField
                    placeholderText: "IP地址"
                    text: "192.168.0.65"
                    Layout.preferredWidth: 130
                    Layout.minimumWidth: 130
                }
                TextField {
                    id: portField
                    placeholderText: "端口"
                    text: "8000"
                    Layout.preferredWidth: 80
                }
                TextField {
                    id: userField
                    placeholderText: "用户名"
                    text: "admin"
                    Layout.preferredWidth: 100
                }
                TextField {
                    id: passField
                    placeholderText: "密码"
                    text: "ty123456"
                    echoMode: TextInput.Password
                    Layout.preferredWidth: 120
                }
                RowLayout {
                    Label {
                        text: "通道:"
                    }
                    SpinBox {
                        id: channelSpin
                        from: 1
                        to: 64
                        value: controller.channel
                        onValueChanged: controller.channel = value
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: controller.isLoggedIn ? "登出" : "登录"
                        highlighted: true
                        onClicked: {
                            if (controller.isLoggedIn)
                                controller.logout();
                            else
                                controller.login(ipField.text, parseInt(portField.text), userField.text, passField.text);
                        }
                    }
                    Button {
                        text: controller.isPlaying ? "停止" : "播放"
                        enabled: controller.isLoggedIn
                        highlighted: true
                        onClicked: controller.isPlaying ? controller.stopPlay() : controller.startPlay()
                    }
                    Button {
                        text: "拍照"
                        enabled: controller.isPlaying
                        onClicked: controller.capture()
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
                text: "Qt 6.10 | Material Style"
                font.pixelSize: 10
                opacity: 0.6
            }
        }
    }
}
