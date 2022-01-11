/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandui.h"
#include "apppermission.h"
#include "authenticationinapp/authenticationinapp.h"
#include "captiveportal/captiveportaldetection.h"
#include "closeeventhandler.h"
#include "commandlineparser.h"
#include "constants.h"
#include "featurelist.h"
#include "features/featureinapppurchase.h"
#include "filterproxymodel.h"
#include "fontloader.h"
#include "l18nstrings.h"
#include "iaphandler.h"
#include "inspector/inspectorwebsocketserver.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "theme.h"

#include <glean.h>
#include <nebula.h>

#ifdef MVPN_LINUX
#  include "eventlistener.h"
#  include "platforms/linux/linuxdependencies.h"
#  include "platforms/linux/linuxappimageprovider.h"
#endif

#ifdef MVPN_MACOS
#  include "platforms/macos/macosmenubar.h"
#  include "platforms/macos/macosstartatbootwatcher.h"
#  include "platforms/macos/macosutils.h"
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androidappimageprovider.h"
#  include "platforms/android/androidutils.h"
#  include "platforms/android/androidwebview.h"
#endif

#ifndef Q_OS_WIN
#  include "signalhandler.h"
#  include <lottie.h>
#endif

#ifdef MVPN_WINDOWS
#  include "eventlistener.h"
#  include "platforms/windows/windowsstartatbootwatcher.h"
#  include "platforms/windows/windowsappimageprovider.h"
#endif

#ifdef MVPN_WASM
#  include "platforms/wasm/wasmwindowcontroller.h"
#endif

#ifdef MVPN_WEBEXTENSION
#  include "server/serverhandler.h"
#endif

#include <QApplication>

namespace {
Logger logger(LOG_MAIN, "CommandUI");
}

CommandUI::CommandUI(QObject* parent) : Command(parent, "ui", "Start the UI.") {
  MVPN_COUNT_CTOR(CommandUI);
}

CommandUI::~CommandUI() { MVPN_COUNT_DTOR(CommandUI); }

int CommandUI::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runQmlApp([&]() {
    QString appName = tokens[0];

    CommandLineParser::Option hOption = CommandLineParser::helpOption();
    CommandLineParser::Option minimizedOption("m", "minimized",
                                              "Start minimized.");
    CommandLineParser::Option startAtBootOption(
        "s", "start-at-boot", "Start at boot (if configured).");
    CommandLineParser::Option testingOption("t", "testing",
                                            "Enable testing mode.");

    QList<CommandLineParser::Option*> options;
    options.append(&hOption);
    options.append(&minimizedOption);
    options.append(&startAtBootOption);
    options.append(&testingOption);

    CommandLineParser clp;
    if (clp.parse(tokens, options, false)) {
      return 1;
    }

    if (!tokens.isEmpty()) {
      return clp.unknownOption(this, appName, tokens[0], options, false);
    }

    if (hOption.m_set) {
      clp.showHelp(this, appName, options, false, false);
      return 0;
    }

    if (testingOption.m_set) {
      Constants::setStaging();
    }

    logger.debug() << "UI starting";

    if (startAtBootOption.m_set) {
      logger.debug() << "Maybe start at boot";

      if (!SettingsHolder::instance()->startAtBoot()) {
        logger.debug() << "We don't need to start at boot.";
        return 0;
      }
    }

    // This object _must_ live longer than MozillaVPN to avoid shutdown crashes.
    QmlEngineHolder engineHolder;
    QQmlApplicationEngine* engine = QmlEngineHolder::instance()->engine();

    Glean::Initialize(engine);

#ifndef MVPN_WINDOWS
    Lottie::initialize(engine, QString(NetworkManager::userAgent()));
#endif

    Nebula::Initialize(engine);

    MozillaVPN vpn;
    vpn.setStartMinimized(minimizedOption.m_set);

    vpn.theme()->loadThemes();

#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX)
    // If there is another instance, the execution terminates here.
    if (!EventListener::checkOtherInstances()) {
      return 0;
    }

    // This class receives communications from other instances.
    EventListener eventListener;
#endif

#ifndef Q_OS_WIN
    // Signal handling for a proper shutdown.
    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested,
                     []() { MozillaVPN::instance()->controller()->quit(); });
#endif

    // Font loader
    FontLoader::loadFonts();

    vpn.initialize();

#ifdef MVPN_MACOS
    MacOSStartAtBootWatcher startAtBootWatcher(
        SettingsHolder::instance()->startAtBoot());
    QObject::connect(SettingsHolder::instance(),
                     &SettingsHolder::startAtBootChanged, &startAtBootWatcher,
                     &MacOSStartAtBootWatcher::startAtBootChanged);

    MacOSUtils::setDockClickHandler();
#endif

#ifdef MVPN_WINDOWS
    WindowsStartAtBootWatcher startAtBootWatcher(
        SettingsHolder::instance()->startAtBoot());

    QObject::connect(SettingsHolder::instance(),
                     &SettingsHolder::startAtBootChanged, &startAtBootWatcher,
                     &WindowsStartAtBootWatcher::startAtBootChanged);
#endif

#ifdef MVPN_LINUX
    // Dependencies - so far, only for linux.
    if (!LinuxDependencies::checkDependencies()) {
      return 1;
    }

    // Register an Image Provider that will resolve "image://app/{id}" for qml
    QQuickImageProvider* provider = new LinuxAppImageProvider(qApp);
    engine->addImageProvider(QString("app"), provider);
#endif

#ifdef MVPN_ANDROID
    // Register an Image Provider that will resolve "image://app/{id}" for qml
    QQuickImageProvider* provider = new AndroidAppImageProvider(qApp);
    engine->addImageProvider(QString("app"), provider);
#endif
#ifdef MVPN_WINDOWS
    // Register an Image Provider that will resolve "image://app/{id}" for qml
    QQuickImageProvider* provider = new WindowsAppImageProvider(qApp);
    engine->addImageProvider(QString("app"), provider);
#endif

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPN", [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNFeatureList",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = FeatureList::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });
    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCaptivePortal",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->captivePortalDetection();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNController",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->controller();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNUser",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->user();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNDeviceModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->deviceModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNFeedbackCategoryModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->feedbackCategoryModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNLicenseModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->licenseModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNSupportCategoryModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->supportCategoryModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNHelpModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->helpModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNServerCountryModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->serverCountryModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNSurveyModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->surveyModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNWhatsNewModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->whatsNewModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCurrentServer",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->currentServer();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNConnectionHealth",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->connectionHealth();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNReleaseMonitor",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->releaseMonitor();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNTheme",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->theme();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNLocalizer",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = Localizer::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNAppPermissions",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = AppPermission::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNSettings",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = SettingsHolder::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNConnectionData",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->connectionDataHolder();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNStatusIcon",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->statusIcon();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCloseEventHandler",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->closeEventHandler();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

#ifdef MVPN_ANDROID
    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNAndroidUtils",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = AndroidUtils::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterType<AndroidWebView>("Mozilla.VPN", 1, 0, "VPNAndroidWebView");
#endif

    if (FeatureInAppPurchase::instance()->isSupported()) {
      qmlRegisterSingletonType<MozillaVPN>(
          "Mozilla.VPN", 1, 0, "VPNIAP",
          [](QQmlEngine*, QJSEngine*) -> QObject* {
            QObject* obj = IAPHandler::instance();
            QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
            return obj;
          });
    }

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNAuthInApp",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = AuthenticationInApp::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNl18n",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = L18nStrings::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNErrorHandler",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = ErrorHandler::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterType<FilterProxyModel>("Mozilla.VPN", 1, 0,
                                      "VPNFilterProxyModel");

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, &vpn,
                     &MozillaVPN::aboutToQuit);

    QObject::connect(
        qApp, &QGuiApplication::commitDataRequest, &vpn,
        []() {
#if QT_VERSION < 0x060000
          qApp->setFallbackSessionManagementEnabled(false);
#endif
          MozillaVPN::instance()->deactivate();
        },
        Qt::DirectConnection);

    QObject::connect(vpn.controller(), &Controller::readyToQuit, &vpn,
                     &MozillaVPN::quit, Qt::QueuedConnection);

    // Here is the main QML file.
    const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
    QObject::connect(
        engine, &QQmlApplicationEngine::objectCreated, qApp,
        [url](QObject* obj, const QUrl& objUrl) {
          if (!obj && url == objUrl) {
            logger.error() << "Failed to load " << objUrl.toString();
            QGuiApplication::exit(-1);
          }
        },
        Qt::QueuedConnection);
    engine->load(url);

    NotificationHandler* notificationHandler =
        NotificationHandler::create(&engineHolder);

    QObject::connect(vpn.controller(), &Controller::stateChanged,
                     notificationHandler,
                     &NotificationHandler::showNotification);

#ifdef MVPN_MACOS
    MacOSMenuBar menuBar;
    menuBar.initialize();

    QObject::connect(&vpn, &MozillaVPN::stateChanged, &menuBar,
                     &MacOSMenuBar::controllerStateChanged);

    QObject::connect(vpn.controller(), &Controller::stateChanged, &menuBar,
                     &MacOSMenuBar::controllerStateChanged);

#endif

    QObject::connect(Localizer::instance(), &Localizer::codeChanged, []() {
      logger.debug() << "Retranslating";
      QmlEngineHolder::instance()->engine()->retranslate();
      NotificationHandler::instance()->retranslate();
      L18nStrings::instance()->retranslate();

#ifdef MVPN_MACOS
      MacOSMenuBar::instance()->retranslate();
#endif

#ifdef MVPN_WASM
      WasmWindowController::instance()->retranslate();
#endif

      MozillaVPN::instance()->serverCountryModel()->retranslate();
    });

    if (!Constants::inProduction()) {
      InspectorWebSocketServer* inspectWebSocketServer =
          new InspectorWebSocketServer(qApp);
      QObject::connect(vpn.controller(), &Controller::readyToQuit,
                       inspectWebSocketServer,
                       &InspectorWebSocketServer::close);
    }

#ifdef MVPN_WASM
    WasmWindowController wasmWindowController;
#endif

#ifdef MVPN_WEBEXTENSION
    ServerHandler serverHandler;
    QObject::connect(vpn.controller(), &Controller::readyToQuit, &serverHandler,
                     &ServerHandler::close);
#endif

    // Let's go.
    return qApp->exec();
  });
}

static Command::RegistrationProxy<CommandUI> s_commandUI;
