#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include <qwindowdefs.h>
#include "HCNetSDK.h"

class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // 属性声明 - 在QML中读取和绑定
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loggedInChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int channel READ channel WRITE setChannel NOTIFY channelChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    bool isLoggedIn() const { return m_isLoggedIn; }
    bool isPlaying() const { return m_isPlaying; }
    QString statusMessage() const { return m_statusMessage; }
    int channel() const { return m_channel; }
    void setChannel(int ch) { if (m_channel != ch) { m_channel = ch; emit channelChanged(); } }

    // Q_INVOKABLE 方法可在QML中调用
    Q_INVOKABLE bool login(const QString& ip, int port, const QString& user, const QString& password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void startPlay(QObject* window);
    Q_INVOKABLE void stopPlay();
    Q_INVOKABLE void capture();
    Q_INVOKABLE void setSDKLog(bool enable);

signals:
    void loggedInChanged();
    void playingChanged();
    void statusMessageChanged();
    void channelChanged();

private:
    void setStatus(const QString& msg);

    bool m_isLoggedIn = false;
    bool m_isPlaying = false;
    int m_channel = 1;
    QString m_statusMessage = "就绪";

    LONG m_lUserID = -1;
    LONG m_lRealHandle = -1;
};

#endif // APPCONTROLLER_H
