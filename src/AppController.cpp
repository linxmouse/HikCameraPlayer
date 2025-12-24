#include "AppController.h"
#include <QDebug>
#include <QDateTime>
#include <QQuickWindow>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QVideoFrameFormat>
#include <QFile>
#include <QMutexLocker>

// 初始化静态成员
QMap<LONG, AppController*> AppController::s_instances;
QMutex AppController::s_mutex;

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
    if (!NET_DVR_Init())
    {
        qDebug() << "NET_DVR_Init failed, error code:" << NET_DVR_GetLastError();
        setStatus("SDK 初始化失败");
    } else
    {
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
        char logDir[] = "./sdk_log/";
        NET_DVR_SetLogToFile(3, logDir, true);
        qDebug() << "SDK Log enabled, path: ./sdk_log/";
    }
}

bool AppController::login(const QString& ip, int port, const QString& user, const QString& password)
{
    qDebug() << "Login attempt:" << ip << port << user;
    if (m_lUserID >= 0)
    {
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

void CALLBACK AppController::RealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    AppController* self = static_cast<AppController*>(pUser);
    if (!self) return;

    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: // 系统头
        if (self->m_nPort >= 0) {
            PlayM4_Stop(self->m_nPort);
            PlayM4_CloseStream(self->m_nPort);
            
            QMutexLocker locker(&s_mutex);
            s_instances.remove(self->m_nPort);
            PlayM4_FreePort(self->m_nPort);
            self->m_nPort = -1;
        }

        if (!PlayM4_GetPort(&self->m_nPort)) {
            break;
        }

        {
            QMutexLocker locker(&s_mutex);
            s_instances.insert(self->m_nPort, self);
        }

        // 设置流模式为实时流
        PlayM4_SetStreamOpenMode(self->m_nPort, STREAME_REALTIME);

        if (!PlayM4_OpenStream(self->m_nPort, pBuffer, dwBufSize, 1024 * 1024)) {
            break;
        }

        // 设置解码回调
        if (!PlayM4_SetDecCallBack(self->m_nPort, DecCBFun)) {
            break;
        }

        if (!PlayM4_Play(self->m_nPort, NULL)) {
            break;
        }
        break;

    case NET_DVR_STREAMDATA: // 码流数据
        if (self->m_nPort >= 0) {
            if (!PlayM4_InputData(self->m_nPort, pBuffer, dwBufSize)) {
                // qDebug() << "PlayM4_InputData failed";
            }
        }
        break;
    }
}

void CALLBACK AppController::DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
    AppController* self = nullptr;
    {
        QMutexLocker locker(&s_mutex);
        self = s_instances.value(nPort, nullptr);
    }

    if (!self || !self->m_videoSink) return;

    // 只处理视频数据 (YV12)
    if (pFrameInfo->nType == T_YV12)
    {
        // 检查是否需要抓图
        bool captureThis = false;
        QString path;
        {
            QMutexLocker locker(&s_mutex);
            if (self->m_captureNextFrame)
            {
                captureThis = true;
                path = self->m_pendingCapturePath;
                self->m_captureNextFrame = false;
            }
        }

        if (captureThis) 
        {
            // 使用 PlayM4_ConvertToJpegFile 直接将 YUV 缓冲转换为 JPEG 文件
            if (PlayM4_ConvertToJpegFile(pBuf, nSize, pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nType, path.toLocal8Bit().data()))
            {
                self->setStatus("拍照成功: " + QFileInfo(path).fileName());
            } else
            {
                self->setStatus("拍照失败 (转换失败)");
            }
        }

        QSize size(pFrameInfo->nWidth, pFrameInfo->nHeight);
        QVideoFrameFormat format(size, QVideoFrameFormat::Format_YV12);
        QVideoFrame frame(format);
        if (frame.map(QVideoFrame::WriteOnly))
        {
            uchar* yPtr = frame.bits(0);
            uchar* vPtr = frame.bits(1);
            uchar* uPtr = frame.bits(2);        
            int yStride = frame.bytesPerLine(0);
            int vStride = frame.bytesPerLine(1);
            int uStride = frame.bytesPerLine(2);        
            int width = size.width();
            int height = size.height();           
            // 拷贝 Y 平面
            const char* srcY = pBuf;
            for (int i = 0; i < height; ++i)
            {
                memcpy(yPtr + i * yStride, srcY + i * width, width);
            }       
            // 拷贝 V 平面 (YV12 中 V 在 U 前面)
            const char* srcV = pBuf + width * height;
            for (int i = 0; i < height / 2; ++i)
            {
                memcpy(vPtr + i * vStride, srcV + i * (width / 2), width / 2);
            }        
            // 拷贝 U 平面
            const char* srcU = srcV + (width * height / 4);
            for (int i = 0; i < height / 2; ++i)
            {
                memcpy(uPtr + i * uStride, srcU + i * (width / 2), width / 2);
            }
            
            frame.unmap();
            self->m_videoSink->setVideoFrame(frame);
        }
    }
}

void AppController::startPlay()
{
    if (m_lUserID < 0)
    {
        setStatus("请先登录!");
        return;
    }

    if (m_lRealHandle >= 0)
    {
        stopPlay();
    }
    qDebug() << "Starting play with callback, channel:" << m_channel;
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = NULL; 
    struPlayInfo.lChannel = m_channel;
    struPlayInfo.dwStreamType = 0;   // 主码流
    struPlayInfo.dwLinkMode = 0;     // TCP
    struPlayInfo.bBlocked = 1;       // 阻塞

    m_lRealHandle = NET_DVR_RealPlay_V40(m_lUserID, &struPlayInfo, RealDataCallBack, this);
    if (m_lRealHandle < 0)
    {
        DWORD err = NET_DVR_GetLastError();
        qDebug() << "NET_DVR_RealPlay_V40 failed, error code:" << err;
        setStatus(QString("播放失败, 错误码: %1").arg(err));
        return;
    }

    m_isPlaying = true;
    emit playingChanged();
    setStatus(QString("正在播放通道 %1 (回调模式)...").arg(m_channel));
}

void AppController::stopPlay()
{
    if (m_lRealHandle >= 0)
    {
        NET_DVR_StopRealPlay(m_lRealHandle);
        m_lRealHandle = -1;
    }
    if (m_nPort >= 0)
    {
        PlayM4_Stop(m_nPort);
        PlayM4_CloseStream(m_nPort);
        
        QMutexLocker locker(&s_mutex);
        s_instances.remove(m_nPort);
        PlayM4_FreePort(m_nPort);
        m_nPort = -1;
    }

    m_isPlaying = false;
    emit playingChanged();
    setStatus("已停止播放");
}

void AppController::capture()
{
    if (m_nPort < 0)
    {
        setStatus("请先开始播放!");
        return;
    } 
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadPath.isEmpty())
    {
        downloadPath = QDir::currentPath();
    } 
    QDir dir(downloadPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    QString fileName = QString("capture_%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString fullPath = dir.absoluteFilePath(fileName);
    {
        QMutexLocker locker(&s_mutex);
        m_captureNextFrame = true;
        m_pendingCapturePath = fullPath;
    }
    
    setStatus("正在抓图...");
}

bool AppController::ptzControl(int command, bool stop, int speed)
{
    if (m_lRealHandle < 0) {
        setStatus("请先开始播放!");
        return false;
    }

    // stop 为 true 表示停止动作，dwStop 参数为 1；stop 为 false 表示开始动作，dwStop 参数为 0
    DWORD dwStop = stop ? 1 : 0;
    
    // 使用 NET_DVR_PTZControlWithSpeed 来支持速度控制
    if (!NET_DVR_PTZControlWithSpeed(m_lRealHandle, (DWORD)command, dwStop, (DWORD)speed)) {
        DWORD err = NET_DVR_GetLastError();
        qDebug() << "NET_DVR_PTZControlWithSpeed failed, command:" << command << "stop:" << dwStop << "error:" << err;
        setStatus(QString("云台控制失败, 错误码: %1").arg(err));
        return false;
    }

    QString cmdName;
    switch (command) {
        case ZoomIn: cmdName = "拉近"; break;
        case ZoomOut: cmdName = "拉远"; break;
        case FocusNear: cmdName = "聚焦近"; break;
        case FocusFar: cmdName = "聚焦远"; break;
        default: cmdName = QString("命令 %1").arg(command); break;
    }

    if (stop) {
        setStatus(QString("停止 %1").arg(cmdName));
    } else {
        setStatus(QString("正在 %1...").arg(cmdName));
    }

    return true;
}

void AppController::setStatus(const QString& msg)
{
    m_statusMessage = msg;
    emit statusMessageChanged();
}
