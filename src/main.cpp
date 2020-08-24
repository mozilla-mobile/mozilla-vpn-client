#include "mozillavpn.h"

#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QScopedPointer<MozillaVPN> mozillaVPN(new MozillaVPN());
    mozillaVPN->initialize(argc, argv);

    QQmlApplicationEngine engine;
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPN", mozillaVPN.get());
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPNController", mozillaVPN->controller());

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
