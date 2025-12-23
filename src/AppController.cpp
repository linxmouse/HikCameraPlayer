#include "AppController.h"
#include <QDebug>
#include <QDateTime>
#include <QQuickWindow>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

// 异常回调函数
void CALLBACK ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser)
{
    char* pMsg = nullptr;
    switch (dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
        pMsg = (char*)"Reconnect";
        break;
    default:
        pMsg = (char*)"Other Exception";
        break;
    }
    qDebug() << "SDK Exception:" << pMsg << "Type:" << dwType << "UserID:" << lUserID << "Handle:" << lHandle;
}

AppController::AppController(QObject* parent): QObject(parent) 
{
    // 初始化 SDK
    if (!NET_DVR_Init()) {
        qDebug() << "NET_DVR_Init failed, error code:" << NET_DVR_GetLastError();
        setStatus("SDK 初始化失败");
    } else {
        qDebug() << "NET_DVR_Init success";
        // 设置异常回调
        NET_DVR_SetExceptionCallBack_V30(0, NULL, ExceptionCallBack, NULL);
        // 默认开启日志
        setSDKLog(true);
    }
}

AppController::~AppController()
{
    stopPlay();
    logout();
    NET_DVR_Cleanup();
}

void AppController::setSDKLog(bool enable)
{
    if (enable) {
        // 设置日志路径和级别
        // 注意：NET_DVR_SetLogToFile 的第二个参数是 char*，需要非 const 指针
        char logDir[] = "./sdk_log/";
        NET_DVR_SetLogToFile(3, logDir, true);
        qDebug() << "SDK Log enabled, path: ./sdk_log/";
    }
}

bool AppController::login(const QString& ip, int port, const QString& user, const QString& password)
{
    qDebug() << "Login attempt:" << ip << port << user;

    if (m_lUserID >= 0) {
        logout();
    }

    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = false;
    strcpy_s(struLoginInfo.sDeviceAddress, ip.toStdString().c_str());
    struLoginInfo.wPort = (WORD)port;
    strcpy_s(struLoginInfo.sUserName, user.toStdString().c_str());
    strcpy_s(struLoginInfo.sPassword, password.toStdString().c_str());

    NET_DVR_DEVICEINFO_V40 struDeviceInfo = {0};
    m_lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfo);

    if (m_lUserID < 0) {
        DWORD err = NET_DVR_GetLastError();
        qDebug() << "NET_DVR_Login_V40 failed, error code:" << err;
        setStatus(QString("登录失败, 错误码: %1").arg(err));
        return false;
    }

    m_isLoggedIn = true;
    emit loggedInChanged();
    setStatus(QString("已连接到 %1:%2").arg(ip).arg(port));
    return true;
}

void AppController::logout()
{
    if (m_lUserID >= 0) {
        NET_DVR_Logout(m_lUserID);
        m_lUserID = -1;
    }
    if (m_isPlaying)
    {
        m_isPlaying = false;
        emit playingChanged();
    }
    m_isLoggedIn = false;
    emit loggedInChanged();
    setStatus("已登出");
}

void AppController::startPlay(QObject* window)
{
    if (m_lUserID < 0) {
        setStatus("请先登录!");
        return;
    }

    if (!window) {
        setStatus("无效的播放窗口!");
        return;
    }

    // 强制显示窗口并处理事件，确保 winId 有效
    window->setProperty("visible", true);
    QCoreApplication::processEvents();

    QQuickWindow* qmlWindow = qobject_cast<QQuickWindow*>(window);
    if (!qmlWindow) {
        setStatus("无法获取窗口对象!");
        return;
    }

    WId winId = qmlWindow->winId();
    if (winId == 0) {
        setStatus("窗口句柄尚未创建!");
        return;
    }

    if (m_lRealHandle >= 0) {
        stopPlay();
    }

    qDebug() << "Starting play on window handle:" << (void*)winId << "channel:" << m_channel;

    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = (HWND)winId;
    struPlayInfo.lChannel = m_channel;
    struPlayInfo.dwStreamType = 0;   // 主码流
    struPlayInfo.dwLinkMode = 0;     // TCP
    struPlayInfo.bBlocked = 1;       // 阻塞

    m_lRealHandle = NET_DVR_RealPlay_V40(m_lUserID, &struPlayInfo, NULL, NULL);

    if (m_lRealHandle < 0) {
        DWORD err = NET_DVR_GetLastError();
        qDebug() << "NET_DVR_RealPlay_V40 failed, error code:" << err;
        setStatus(QString("播放失败, 错误码: %1").arg(err));
        return;
    }

    m_isPlaying = true;
    emit playingChanged();
    setStatus(QString("正在播放通道 %1...").arg(m_channel));
}

void AppController::stopPlay()
{
    if (m_lRealHandle >= 0) {
        NET_DVR_StopRealPlay(m_lRealHandle);
        m_lRealHandle = -1;
    }
    m_isPlaying = false;
    emit playingChanged();
    setStatus("已停止播放");
}

void AppController::capture()
{
    if (m_lRealHandle < 0) {
        setStatus("请先开始播放!");
        return;
    }
    
    // 获取用户下载目录
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadPath.isEmpty()) {
        downloadPath = QDir::currentPath();
    }
    
    // 确保目录存在
    QDir dir(downloadPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString fileName = QString("capture_%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString fullPath = dir.absoluteFilePath(fileName);

    if (NET_DVR_CapturePicture(m_lRealHandle, fullPath.toLocal8Bit().data())) {
        setStatus("拍照成功: " + fileName);
        qDebug() << "Picture saved to:" << fullPath;
    } else {
        setStatus(QString("拍照失败, 错误码: %1").arg(NET_DVR_GetLastError()));
    }
}

void AppController::setStatus(const QString& msg)
{
    m_statusMessage = msg;
    emit statusMessageChanged();
}
