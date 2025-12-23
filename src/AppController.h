#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include <qwindowdefs.h>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMap>
#include <QMutex>
#include <QFileInfo>
#include "HCNetSDK.h"
#include "plaympeg4.h"

class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // 属性声明 - 在QML中读取和绑定
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loggedInChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int channel READ channel WRITE setChannel NOTIFY channelChanged)
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    bool isLoggedIn() const { return m_isLoggedIn; }
    bool isPlaying() const { return m_isPlaying; }
    QString statusMessage() const { return m_statusMessage; }
    int channel() const { return m_channel; }
    void setChannel(int ch) { if (m_channel != ch) { m_channel = ch; emit channelChanged(); } }

    QVideoSink* videoSink() const { return m_videoSink; }
    void setVideoSink(QVideoSink* sink) { if (m_videoSink != sink) { m_videoSink = sink; emit videoSinkChanged(); } }

    // Q_INVOKABLE 方法可在QML中调用
    Q_INVOKABLE bool login(const QString& ip, int port, const QString& user, const QString& password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void startPlay();
    Q_INVOKABLE void stopPlay();
    Q_INVOKABLE void capture();
    Q_INVOKABLE void setSDKLog(bool enable);

signals:
    void loggedInChanged();
    void playingChanged();
    void statusMessageChanged();
    void channelChanged();
    void videoSinkChanged();

private:
    void setStatus(const QString& msg);
    
    // SDK 回调函数
    static void CALLBACK RealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
    static void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);

    // 静态实例注册表，用于在回调中通过 nPort 找回对象指针。
    static QMap<LONG, AppController*> s_instances;
    static QMutex s_mutex;

    bool m_isLoggedIn = false;
    bool m_isPlaying = false;
    int m_channel = 1;
    QString m_statusMessage = "就绪";
    QVideoSink* m_videoSink = nullptr;

    // 拍照相关
    bool m_captureNextFrame = false;
    QString m_pendingCapturePath;

    LONG m_lUserID = -1;
    LONG m_lRealHandle = -1;
    LONG m_nPort = -1; // PlayM4 播放库端口
};

#endif // APPCONTROLLER_H
