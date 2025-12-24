#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/qt/qml/work/dbugs/hikcamera/app_icon.png"));
    
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    // engine.load(QUrl("qrc:/qt/qml/work/dbugs/hikcamera/Main.qml"));
    engine.loadFromModule("work.dbugs.hikcamera", "Main");
    return app.exec();
}
