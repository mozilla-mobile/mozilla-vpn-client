#include "mozillavpn.h"
#include "signalhandler.h"

#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QScopedPointer<MozillaVPN> mozillaVPN(new MozillaVPN());
    mozillaVPN->initialize(argc, argv);

    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested, [&]() {
        mozillaVPN->controller()->quit();
    });

    QQmlApplicationEngine engine;
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPN", mozillaVPN.get());
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPNController", mozillaVPN->controller());
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPNUser", mozillaVPN->user());
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPNDeviceModel", mozillaVPN->deviceModel());
    qmlRegisterSingletonInstance("Mozilla.VPN",
                                 1,
                                 0,
                                 "VPNServerCountryModel",
                                 mozillaVPN->serverCountryModel());
    qmlRegisterSingletonInstance("Mozilla.VPN",
                                 1,
                                 0,
                                 "VPNCurrentServer",
                                 mozillaVPN->currentServer());

    QObject::connect(mozillaVPN->controller(),
                     &Controller::readyToQuit,
                     &app,
                     QCoreApplication::quit,
                     Qt::QueuedConnection);

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
