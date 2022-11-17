/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandui.h"

#include <glean.h>
#include <lottie.h>
#include <nebula.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "addons/manager/addonmanager.h"
#include "appconstants.h"
#include "apppermission.h"
#include "authenticationinapp/authenticationinapp.h"
#include "captiveportal/captiveportaldetection.h"
#include "commandlineparser.h"
#include "connectionbenchmark/connectionbenchmark.h"
#include "connectionhealth.h"
#include "controller.h"
#include "feature.h"
#include "fontloader.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "ipaddresslookup.h"
#include "models/devicemodel.h"
#include "models/feedbackcategorymodel.h"
#include "models/licensemodel.h"
#include "models/servercountrymodel.h"
#include "models/subscriptiondata.h"
#include "models/supportcategorymodel.h"
#include "models/user.h"
// Relative path is required here,
// otherwise this gets confused with the Glean.js implementation
#include "glean/mzglean.h"
#include "gleandeprecated.h"
#include "i18nstrings.h"
#include "imageproviderfactory.h"
#include "inspector/inspectorhandler.h"
#include "keyregenerator.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "models/featuremodel.h"
#include "models/recentconnections.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "notificationhandler.h"
#include "productshandler.h"
#include "profileflow.h"
#include "purchasehandler.h"
#include "qmlengineholder.h"
#include "releasemonitor.h"
#include "serverlatency.h"
#include "settingsholder.h"
#include "telemetry.h"
#include "telemetry/gleansample.h"
#include "temporarydir.h"
#include "theme.h"
#include "tutorial/tutorial.h"
#include "update/updater.h"
#include "urlopener.h"

#ifdef MZ_DEBUG
#  include <QQmlDebuggingEnabler>
#endif

#ifdef MZ_LINUX
#  include "eventlistener.h"
#  include "platforms/linux/linuxdependencies.h"
#endif

#ifdef MZ_MACOS
#  include "platforms/macos/macosmenubar.h"
#  include "platforms/macos/macosstartatbootwatcher.h"
#  include "platforms/macos/macosutils.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidutils.h"
#endif

#ifndef Q_OS_WIN
#  include "signalhandler.h"
#endif

#ifdef SENTRY_ENABLED
#  include "sentry/sentryadapter.h"
#endif

#ifdef MZ_WINDOWS
#  include <windows.h>

#  include <iostream>

#  include "eventlistener.h"
#  include "platforms/windows/windowsappimageprovider.h"
#  include "platforms/windows/windowsstartatbootwatcher.h"
#endif

#ifdef MZ_WASM
#  include "platforms/wasm/wasmnetworkrequest.h"
#  include "platforms/wasm/wasmwindowcontroller.h"
#endif

#ifdef MVPN_WEBEXTENSION
#  include "server/serverhandler.h"
#endif

#include <QApplication>

namespace {
Logger logger("CommandUI");
}

CommandUI::CommandUI(QObject* parent) : Command(parent, "ui", "Start the UI.") {
  MZ_COUNT_CTOR(CommandUI);
}

CommandUI::~CommandUI() { MZ_COUNT_DTOR(CommandUI); }

int CommandUI::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runQmlApp([&]() {
    MozillaVPN vpn;
    vpn.telemetry()->startTimeToFirstScreenTimer();

    QString appName = tokens[0];

    CommandLineParser::Option hOption = CommandLineParser::helpOption();
    CommandLineParser::Option minimizedOption("m", "minimized",
                                              "Start minimized.");
    CommandLineParser::Option startAtBootOption(
        "s", "start-at-boot", "Start at boot (if configured).");
    CommandLineParser::Option testingOption("t", "testing",
                                            "Enable testing mode.");
    CommandLineParser::Option updateOption(
        "u", "updated", "This execution completes an update flow.");

    QList<CommandLineParser::Option*> options;
    options.append(&hOption);
    options.append(&minimizedOption);
    options.append(&startAtBootOption);
    options.append(&testingOption);
    options.append(&updateOption);

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
      AppConstants::setStaging();
    }

    logger.info() << "MozillaVPN" << Constants::versionString();
    logger.info() << "User-Agent:" << NetworkManager::userAgent();

    logger.debug() << "UI starting";

    if (startAtBootOption.m_set || qgetenv("MVPN_STARTATBOOT") == "1") {
      logger.debug() << "Maybe start at boot";

      if (!SettingsHolder::instance()->startAtBoot()) {
        logger.debug() << "We don't need to start at boot.";
        return 0;
      }

      if (SettingsHolder::instance()->token().isEmpty()) {
        logger.debug() << "The user is not logged in.";
        return 0;
      }
    }

#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
    // If there is another instance, the execution terminates here.
    if (!EventListener::checkOtherInstances()) {
      return 0;
    }

    // This class receives communications from other instances.
    EventListener eventListener;
#endif

#ifdef MZ_WINDOWS
#  ifdef MZ_DEBUG
    // Allocate a console to view log output in debug mode on windows
    if (AllocConsole()) {
      FILE* unusedFile;
      freopen_s(&unusedFile, "CONOUT$", "w", stdout);
      freopen_s(&unusedFile, "CONOUT$", "w", stderr);
      std::cout.clear();
      std::clog.clear();
      std::cerr.clear();
    }
#  endif
#endif

#ifdef MZ_DEBUG
    // This enables the qt-creator qml debugger on debug builds.:
    // Go to QtCreator: Debug->Start Debugging-> Attach to QML port
    // Port is 1234.
    // Note: Qt creator only will use localhost:port so tunnel any external
    // device to there i.e on android $adb forward tcp:1234 tcp:1234

    // We need to create the qmldebug server before the engine is created.
    QQmlDebuggingEnabler enabler;
    bool ok = enabler.startTcpDebugServer(
        1234, QQmlDebuggingEnabler::StartMode::DoNotWaitForClient, "0.0.0.0");
    if (ok) {
      logger.debug() << "Started QML Debugging server on 0.0.0.0:1234";
    } else {
      logger.error() << "Failed to start QML Debugging";
    }
#endif
#ifdef MZ_ANDROID
    // https://bugreports.qt.io/browse/QTBUG-82617
    // Currently there is a crash happening on exit with Huawei devices.
    // Until this is fixed, setting this variable is the "official" workaround.
    // We certainly should look at this once 6.6 is out.
#  if QT_VERSION >= 0x060600
#    error We have forgotten to remove this Huawei hack!
#  endif
    if (AndroidUtils::GetManufacturer() == "Huawei") {
      qputenv("QT_ANDROID_NO_EXIT_CALL", "1");
    }
#endif
    // This object _must_ live longer than MozillaVPN to avoid shutdown crashes.
    QQmlApplicationEngine* engine = new QQmlApplicationEngine();
    QmlEngineHolder engineHolder(engine);

    // TODO pending #3398
    QQmlContext* ctx = engine->rootContext();
    ctx->setContextProperty("QT_QUICK_BACKEND", qgetenv("QT_QUICK_BACKEND"));

    // Glean.js
    Glean::Initialize(engine);
    // Glean.rs
    MZGlean::initialize();

    Lottie::initialize(engine, QString(NetworkManager::userAgent()));
    Nebula::Initialize(engine);
    I18nStrings::initialize();

    // Cleanup previous temporary files.
    TemporaryDir::cleanupAll();

    vpn.setStartMinimized(minimizedOption.m_set ||
                          (qgetenv("MVPN_MINIMIZED") == "1"));

    if (updateOption.m_set) {
      mozilla::glean::sample::update_step.record(
          mozilla::glean::sample::UpdateStepExtra{
              ._state =
                  QVariant::fromValue(Updater::ApplicationRestartedAfterUpdate)
                      .toString()});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::updateStep,
          {{"state",
            QVariant::fromValue(Updater::ApplicationRestartedAfterUpdate)
                .toString()}});
    }

#ifndef Q_OS_WIN
    // Signal handling for a proper shutdown.
    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested,
                     []() { MozillaVPN::instance()->controller()->quit(); });
#endif

    // Font loader
    FontLoader::loadFonts();

    vpn.initialize();

#ifdef MZ_MACOS
    MacOSStartAtBootWatcher startAtBootWatcher;
    MacOSUtils::setDockClickHandler();
#endif

#ifdef MZ_WINDOWS
    WindowsStartAtBootWatcher startAtBootWatcher;
#endif

#if defined(MZ_LINUX) && !defined(MZ_FLATPAK)
    // Dependencies - so far, only for linux.
    if (!LinuxDependencies::checkDependencies()) {
      return 1;
    }
#endif

    QQuickImageProvider* provider = ImageProviderFactory::create(qApp);
    if (provider) {
      engine->addImageProvider(QString("app"), provider);
    }
    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPN", [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNFeatureList",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = FeatureModel::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "MZGleanDeprecated",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = GleanDeprecated::instance();
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

#ifdef SENTRY_ENABLED
    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCrashReporter",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = SentryAdapter::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });
#endif

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
          QObject* obj = LicenseModel::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNRecentConnectionsModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = RecentConnections::instance();
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
        "Mozilla.VPN", 1, 0, "VPNServerCountryModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->serverCountryModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNSubscriptionData",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->subscriptionData();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNProfileFlow",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->profileFlow();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCurrentServer",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->serverData();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNServerLatency",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->serverLatency();
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
        [](QQmlEngine*, QJSEngine* engine) -> QObject* {
          Theme* theme = MozillaVPN::instance()->theme();
          theme->initialize(engine);
          QQmlEngine::setObjectOwnership(theme, QQmlEngine::CppOwnership);
          return theme;
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
        "Mozilla.VPN", 1, 0, "VPNConnectionBenchmark",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->connectionBenchmark();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNIPAddressLookup",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance()->ipAddressLookup();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNNavigator",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = Navigator::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

#ifdef MZ_ANDROID
    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNAndroidUtils",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = AndroidUtils::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });
#endif

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNPurchase",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = PurchaseHandler::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    if (!Feature::get(Feature::Feature_webPurchase)->isSupported()) {
      qmlRegisterSingletonType<MozillaVPN>(
          "Mozilla.VPN", 1, 0, "VPNProducts",
          [](QQmlEngine*, QJSEngine*) -> QObject* {
            QObject* obj = ProductsHandler::instance();
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
        "Mozilla.VPN", 1, 0, "VPNI18n",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = I18nStrings::instance();
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

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNTutorial",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = Tutorial::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNAddonManager",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = AddonManager::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNUrlOpener",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = UrlOpener::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "GleanPings",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = __DONOTUSE__GleanPings::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "Glean", [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = __DONOTUSE__GleanMetrics::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

#if MVPN_IOS && QT_VERSION >= 0x060000 && QT_VERSION < 0x060300
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, &vpn,
                     &MozillaVPN::quit);
#else
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, &vpn, [] {
      // Submit the main ping one last time.
      mozilla::glean_pings::Main.submit();
      // During shutdown Glean will attempt to finish all tasks
      // and submit all enqueued pings (including the one we
      // just sent).
      MZGlean::shutdown();

      emit MozillaVPN::instance()->aboutToQuit();
    });
#endif

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
    engine->load(url);
    if (!engineHolder.hasWindow()) {
      logger.error() << "Failed to load " << url.toString();
      return -1;
    }

    NotificationHandler* notificationHandler =
        NotificationHandler::create(&engineHolder);

    QObject::connect(vpn.controller(), &Controller::stateChanged,
                     notificationHandler,
                     &NotificationHandler::showNotification);

#ifdef MZ_MACOS
    MacOSMenuBar menuBar;
    menuBar.initialize();

    QObject::connect(&vpn, &MozillaVPN::stateChanged, &menuBar,
                     &MacOSMenuBar::controllerStateChanged);

    QObject::connect(vpn.controller(), &Controller::stateChanged, &menuBar,
                     &MacOSMenuBar::controllerStateChanged);

#endif

    QObject::connect(
        SettingsHolder::instance(), &SettingsHolder::languageCodeChanged, []() {
          logger.debug() << "Retranslating";
          QmlEngineHolder::instance()->engine()->retranslate();
          NotificationHandler::instance()->retranslate();
          I18nStrings::instance()->retranslate();
          AddonManager::instance()->retranslate();

#ifdef MZ_MACOS
          MacOSMenuBar::instance()->retranslate();
#endif

#ifdef MZ_WASM
          WasmWindowController::instance()->retranslate();
#endif

          MozillaVPN::instance()->serverCountryModel()->retranslate();
          MozillaVPN::instance()->serverData()->retranslate();
        });

    InspectorHandler::initialize();

#ifdef MZ_WASM
    WasmWindowController wasmWindowController;

    NetworkRequest::setRequestHandler(WasmNetworkRequest::deleteResource,
                                      WasmNetworkRequest::getResource,
                                      WasmNetworkRequest::postResource,
                                      WasmNetworkRequest::postResourceIODevice);
#endif

#ifdef MVPN_WEBEXTENSION
    ServerHandler serverHandler;
    QObject::connect(vpn.controller(), &Controller::readyToQuit, &serverHandler,
                     &ServerHandler::close);
#endif

    KeyRegenerator keyRegenerator;
    // Let's go.
    return qApp->exec();
  });
}

static Command::RegistrationProxy<CommandUI> s_commandUI;
