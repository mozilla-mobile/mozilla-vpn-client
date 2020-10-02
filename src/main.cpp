/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "logger.h"
#include "mozillavpn.h"
#include "signalhandler.h"
#include "systemtrayhandler.h"

#ifdef __linux__
#include "platforms/linux/linuxdependencies.h"
#endif

#ifdef MACOS_INTEGRATION
#include "platforms/macos/macosutils.h"
#endif

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QSystemTrayIcon>
#include <QWindow>

int main(int argc, char *argv[])
{
    // Our logging system.
    qInstallMessageHandler(Logger::messageHandler);

    // The application.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QIcon icon("://resources/logo.png");
    app.setWindowIcon(icon);

    // Dependencies - so far, only for linux.
#ifdef __linux__
    if (!LinuxDependencies::checkDependencies()) {
        return 1;
    }
#endif

#ifdef MACOS_INTEGRATION
    MacOSUtils::enableLoginItem();
#endif

    // Signal handling for a proper shutdown.
    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested, [&]() {
        MozillaVPN::instance()->controller()->quit();
    });

    // Create the QML engine and expose a few internal objects.
    QQmlApplicationEngine engine;

    MozillaVPN::createInstance(&app, &engine);

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

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNLocalizer", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->localizer();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNSettings", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->settingsHolder();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNConnectionData", [](QQmlEngine *, QJSEngine *) -> QObject * {
            QObject *obj = MozillaVPN::instance()->connectionDataHolder();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
        });

    QObject::connect(MozillaVPN::instance()->settingsHolder(),
                     &SettingsHolder::languageCodeChanged,
                     [engine = &engine](const QString &languageCode) {
                         qDebug() << "Storing the languageCode:" << languageCode;
                         MozillaVPN::instance()->localizer()->loadLanguage(languageCode);
                         engine->retranslate();
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

    // System tray icon and messages.
    SystemTrayHandler systemTrayHandler(icon, &app);
    systemTrayHandler.show();

    QObject::connect(&systemTrayHandler, &QSystemTrayIcon::activated, [engine = &engine]() {
        QObject *rootObject = engine->rootObjects().first();
        QWindow *window = qobject_cast<QWindow *>(rootObject);
        Q_ASSERT(window);

        window->show();
        window->raise();
        window->requestActivate();
    });

    QObject::connect(&systemTrayHandler, &SystemTrayHandler::quit, []() {
        MozillaVPN::instance()->controller()->quit();
    });

    QObject::connect(MozillaVPN::instance()->controller(),
                     &Controller::stateChanged,
                     &systemTrayHandler,
                     &SystemTrayHandler::controllerStateChanged);

    QObject::connect(MozillaVPN::instance()->captivePortalDetection(),
                     &CaptivePortalDetection::captivePortalDetected,
                     [systemTrayHandler = &systemTrayHandler]() {
                         systemTrayHandler->captivePortalNotificationRequested();
                     });

    // Let's go.
    return app.exec();
}
