/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandui.h"
#include "captiveportal/captiveportaldetection.h"
#include "closeeventhandler.h"
#include "commandlineparser.h"
#include "fontloader.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "signalhandler.h"
#include "systemtrayhandler.h"

#ifdef MVPN_LINUX
#include "platforms/linux/linuxdependencies.h"
#endif

#ifdef MVPN_MACOS
#include "platforms/macos/macosstartatbootwatcher.h"
#include "platforms/macos/macosutils.h"
#endif

#ifdef Q_OS_MAC
#ifndef MVPN_IOS
#include "platforms/macos/macosmenubar.h"
#endif
#endif

#ifdef QT_DEBUG
#include "inspector/inspectorserver.h"
#endif

#ifdef MVPN_ANDROID
#include "platforms/android/androidutils.h"
#include "platforms/android/androidwebview.h"
#endif

#include <QApplication>
#include <QWindow>

#ifdef QT_DEBUG
#include <QLoggingCategory>
#endif

namespace {
Logger logger(LOG_MAIN, "CommandUI");
}

CommandUI::CommandUI() : Command("ui", "Start the UI.") {}

int CommandUI::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());
    return runQmlApp([&]() {
        QString appName = tokens[0];

        CommandLineParser::Option hOption = CommandLineParser::helpOption();
        CommandLineParser::Option minimizedOption("m", "minimized", "Start minimized.");
        CommandLineParser::Option startAtBootOption("s",
                                                    "start-at-boot",
                                                    "Start at boot (if configured).");

        QList<CommandLineParser::Option *> options;
        options.append(&hOption);
        options.append(&minimizedOption);
        options.append(&startAtBootOption);

        CommandLineParser clp;
        if (clp.parse(tokens, options, false)) {
            return 1;
        }

        if (!tokens.isEmpty()) {
            return clp.unknownOption(appName, tokens[0], options, false);
        }

        if (hOption.m_set) {
            clp.showHelp(appName, options, false, false);
            return 0;
        }

        logger.log() << "UI starting";

        if (startAtBootOption.m_set) {
            logger.log() << "Maybe start at boot";

            if (!SettingsHolder::instance()->startAtBoot()) {
                logger.log() << "We don't need to start at boot.";
                return 0;
            }
        }

        MozillaVPN vpn;
        vpn.setStartMinimized(minimizedOption.m_set);

        // Signal handling for a proper shutdown.
        SignalHandler sh;
        QObject::connect(&sh, &SignalHandler::quitRequested, [&]() {
            MozillaVPN::instance()->controller()->quit();
        });

        // Font loader
        FontLoader::loadFonts();

        // Create the QML engine and expose a few internal objects.
        QmlEngineHolder engineHolder;
        QQmlApplicationEngine *engine = QmlEngineHolder::instance()->engine();
        vpn.initialize();

#ifdef MVPN_MACOS
        MacOSStartAtBootWatcher startAtBootWatcher(SettingsHolder::instance()->startAtBoot());
        QObject::connect(SettingsHolder::instance(),
                         &SettingsHolder::startAtBootChanged,
                         &startAtBootWatcher,
                         &MacOSStartAtBootWatcher::startAtBootChanged);

        MacOSUtils::setDockClickHandler();
#endif

#ifdef MVPN_LINUX
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
            "Mozilla.VPN", 1, 0, "VPNHelpModel", [](QQmlEngine *, QJSEngine *) -> QObject * {
                QObject *obj = MozillaVPN::instance()->helpModel();
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
                QObject *obj = Localizer::instance();
                QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
                return obj;
            });

        qmlRegisterSingletonType<MozillaVPN>(
            "Mozilla.VPN", 1, 0, "VPNSettings", [](QQmlEngine *, QJSEngine *) -> QObject * {
                QObject *obj = SettingsHolder::instance();
                QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
                return obj;
            });

        qmlRegisterSingletonType<MozillaVPN>(
            "Mozilla.VPN", 1, 0, "VPNConnectionData", [](QQmlEngine *, QJSEngine *) -> QObject * {
                QObject *obj = MozillaVPN::instance()->connectionDataHolder();
                QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
                return obj;
            });

        qmlRegisterSingletonType<MozillaVPN>(
            "Mozilla.VPN", 1, 0, "VPNStatusIcon", [](QQmlEngine *, QJSEngine *) -> QObject * {
                QObject *obj = MozillaVPN::instance()->statusIcon();
                QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
                return obj;
            });

        qmlRegisterSingletonType<MozillaVPN>(
            "Mozilla.VPN", 1, 0, "VPNCloseEventHandler", [](QQmlEngine *, QJSEngine *) -> QObject * {
                QObject *obj = MozillaVPN::instance()->closeEventHandler();
                QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
                return obj;
            });

#ifdef MVPN_ANDROID
        qmlRegisterSingletonType<MozillaVPN>(
            "Mozilla.VPN", 1, 0, "VPNAndroidUtils", [](QQmlEngine *, QJSEngine *) -> QObject * {
                QObject *obj = AndroidUtils::instance();
                QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
                return obj;
            });

        qmlRegisterType<AndroidWebView>("Mozilla.VPN", 1, 0, "VPNAndroidWebView");
#endif

        QObject::connect(SettingsHolder::instance(),
                         &SettingsHolder::languageCodeChanged,
                         [](const QString &languageCode) {
                             logger.log() << "Storing the languageCode:" << languageCode;
                             Localizer::instance()->loadLanguage(languageCode);
                             QmlEngineHolder::instance()->engine()->retranslate();
                         });

        QObject::connect(vpn.controller(),
                         &Controller::readyToQuit,
                         qApp,
                         QCoreApplication::quit,
                         Qt::QueuedConnection);

        // Here is the main QML file.
        const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
        QObject::connect(
            engine,
            &QQmlApplicationEngine::objectCreated,
            qApp,
            [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl) {
                    QGuiApplication::exit(-1);
                }
            },
            Qt::QueuedConnection);
        engine->load(url);

        SystemTrayHandler systemTrayHandler(qApp);
        systemTrayHandler.show();

        NotificationHandler *notificationHandler = NotificationHandler::create(qApp);

        QObject::connect(&vpn,
                         &MozillaVPN::stateChanged,
                         &systemTrayHandler,
                         &SystemTrayHandler::updateContextMenu);

        QObject::connect(vpn.currentServer(),
                         &ServerData::changed,
                         &systemTrayHandler,
                         &SystemTrayHandler::updateContextMenu);

        QObject::connect(vpn.controller(),
                         &Controller::stateChanged,
                         &systemTrayHandler,
                         &SystemTrayHandler::updateContextMenu);

        QObject::connect(vpn.controller(),
                         &Controller::stateChanged,
                         notificationHandler,
                         &NotificationHandler::showNotification);

#ifdef Q_OS_MAC
#ifndef MVPN_IOS
        MacOSMenuBar menuBar;
        menuBar.initialize();

        QObject::connect(&vpn,
                         &MozillaVPN::stateChanged,
                         &menuBar,
                         &MacOSMenuBar::controllerStateChanged);

        QObject::connect(vpn.controller(),
                         &Controller::stateChanged,
                         &menuBar,
                         &MacOSMenuBar::controllerStateChanged);
#endif
#endif

        QObject::connect(vpn.statusIcon(),
                         &StatusIcon::iconChanged,
                         &systemTrayHandler,
                         &SystemTrayHandler::updateIcon);

        QObject::connect(vpn.captivePortalDetection(),
                         &CaptivePortalDetection::captivePortalDetected,
                         [systemTrayHandler = &systemTrayHandler]() {
                             systemTrayHandler->captivePortalNotificationRequested();
                         });

#ifdef QT_DEBUG
        InspectorServer inspectServer;
        QObject::connect(vpn.controller(),
                         &Controller::readyToQuit,
                         &inspectServer,
                         &InspectorServer::close);
#endif

        // Let's go.
        return qApp->exec();
    });
}

static CommandUI s_commandUI;
