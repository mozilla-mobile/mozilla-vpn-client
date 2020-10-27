/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportal/captiveportaldetection.h"
#include "fontloader.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "signalhandler.h"
#include "systemtrayhandler.h"

#ifdef __linux__
#include "platforms/linux/linuxdependencies.h"
#endif

#ifdef MACOS_INTEGRATION
#include "platforms/macos/macosstartatbootwatcher.h"
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QSystemTrayIcon>
#include <QWindow>

#ifdef QT_DEBUG
#include <QLoggingCategory>
#endif

namespace {
Logger logger(LOG_MAIN, "main");
}

int main(int argc, char *argv[])
{
    // Our logging system.
    qInstallMessageHandler(LogHandler::messageQTHandler);

#ifdef QT_DEBUG
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qml.binding.removal.info=true"));
#endif

    logger.log() << "MozillaVPN" << APP_VERSION;

    // The application.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QIcon icon("://ui/resources/logo-dock.png");

    app.setWindowIcon(icon);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        qtTrId("vpn.main.productDescription"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption minimizedOption(QStringList() << "m"
                                                     << "minimized",
                                        //% "Start minimized"
                                        qtTrId("vpn.main.startMinimized"));
    parser.addOption(minimizedOption);

    QCommandLineOption startAtBootOption(QStringList() << "s"
                                                       << "start-at-boot",
                                        //% "Start at boot (if configured)"
                                        qtTrId("vpn.main.startOnBoot"));
    parser.addOption(startAtBootOption);

    parser.process(app);

    // Signal handling for a proper shutdown.
    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested, [&]() {
        MozillaVPN::instance()->controller()->quit();
    });

    // Font loader
    FontLoader::loadFonts();

    // Create the QML engine and expose a few internal objects.
    QQmlApplicationEngine engine;

    MozillaVPN::createInstance(&app, &engine, parser.isSet(minimizedOption));

    if (parser.isSet(startAtBootOption)) {
        logger.log() << "Maybe start at boot";

        if (!MozillaVPN::instance()->settingsHolder()->startAtBoot()) {
            logger.log() << "We don't need to start at boot.";
            return 0;
        }
    }

#ifdef MACOS_INTEGRATION
    MacOSStartAtBootWatcher startAtBootWatcher(
        MozillaVPN::instance()->settingsHolder()->startAtBoot());
    QObject::connect(MozillaVPN::instance()->settingsHolder(),
                     &SettingsHolder::startAtBootChanged,
                     &startAtBootWatcher,
                     &MacOSStartAtBootWatcher::startAtBootChanged);
#endif

#if defined (__linux__) && !defined (__ANDROID__)
    // Dependencies - so far, only for linux.
    if (!LinuxDependencies::checkDependencies()) {
        return 1;
    }
#endif

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
                         logger.log() << "Storing the languageCode:" << languageCode;
                         MozillaVPN::instance()->localizer()->loadLanguage(languageCode);
                         engine->retranslate();
                     });

    QObject::connect(MozillaVPN::instance()->controller(),
                     &Controller::readyToQuit,
                     &app,
                     QCoreApplication::quit,
                     Qt::QueuedConnection);

    // Here is the main QML file.
    const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
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
    QIcon trayIcon("://ui/resources/logo-tray.svg");
    trayIcon.setIsMask(true);
    SystemTrayHandler systemTrayHandler(trayIcon, &app);
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
