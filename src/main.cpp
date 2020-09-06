#include "logger.h"
#include "mozillavpn.h"
#include "signalhandler.h"

#ifdef __linux__
#include "platforms/linux/wgquickprocess.h"
#endif

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    qInstallMessageHandler(Logger::messageHandler);

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("qrc:/resources/logo.png"));

#ifdef __linux__
    if (!WgQuickProcess::checkDependencies()) {
        return 1;
    }
#endif

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
    qmlRegisterSingletonInstance("Mozilla.VPN",
                                 1,
                                 0,
                                 "VPNConnectionHealth",
                                 mozillaVPN->connectionHealth());
    qmlRegisterSingletonInstance("Mozilla.VPN", 1, 0, "VPNLogger", Logger::instance());

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
                QGuiApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
