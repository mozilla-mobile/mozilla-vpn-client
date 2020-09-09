#include "logger.h"
#include "mozillavpn.h"
#include "signalhandler.h"

#ifdef __linux__
#include "platforms/linux/wgquickdependencies.h"
#endif

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    // Our logging system.
    qInstallMessageHandler(Logger::messageHandler);

    // The application.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("qrc:/resources/logo.png"));

    // Dependencies - so far, only for linux.
#ifdef __linux__
    if (!WgQuickDependencies::checkDependencies()) {
        return 1;
    }
#endif

    // Signal handling for a proper shutdown.
    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested, [&]() {
        MozillaVPN::instance()->controller()->quit();
    });

    // Create the QML engine and expose a few internal objects.
    QQmlApplicationEngine engine;

    // This object creates the MozillaVPN and it deletes it at shutdown.
    // In theory, this is not needed and we can have MozillaVPN as a normal
    // singleton, but I like that, at shutdown, all the memory is correctly
    // released.
    MozillaVPN::Holder holder;

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPN", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNController", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->controller();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNUser", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->user();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNDeviceModel", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->deviceModel();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNServerCountryModel", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->serverCountryModel();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCurrentServer", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->currentServer();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNConnectionHealth", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->connectionHealth();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNLogger", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = Logger::instance();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    QObject::connect(MozillaVPN::instance()->controller(),
                     &Controller::readyToQuit,
                     &app,
                     QCoreApplication::quit,
                     Qt::QueuedConnection);

    // Here is the main QML file.
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
