/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandui.h"

#include <lottie.h>
#include <nebula.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "accessiblenotification.h"
#include "addons/manager/addonmanager.h"
#include "apppermission.h"
#include "commandlineparser.h"
#include "constants.h"
#include "controller.h"
#include "daemon/mock/mockdaemon.h"
#include "fontloader.h"
#include "glean/generated/pings.h"
#include "glean/mzglean.h"
#include "i18nstrings.h"
#include "imageproviderfactory.h"
#include "inspector/inspectorhandler.h"
#include "keyregenerator.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "telemetry.h"
#include "temporarydir.h"

#ifdef MZ_DEBUG
#  include <QQmlDebuggingEnabler>
#endif

#ifdef MZ_LINUX
#  include "eventlistener.h"
#  include "platforms/linux/xdgstartatbootwatcher.h"
#endif

#ifdef MZ_MACOS
#  include "platforms/macos/macosmenubar.h"
#  include "platforms/macos/macosstartatbootwatcher.h"
#  include "platforms/macos/macosutils.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#  include "platforms/android/androidutils.h"
#  include "platforms/android/androidvpnactivity.h"
#endif

#ifndef Q_OS_WIN
#  include "signalhandler.h"
#endif

#ifdef MZ_WINDOWS
#  include <windows.h>

#  include "eventlistener.h"
#  include "platforms/windows/windowsstartatbootwatcher.h"
#endif

#ifdef MZ_WASM
#  include "platforms/wasm/wasmwindowcontroller.h"
#endif

#ifdef MVPN_WEBEXTENSION
#  include "webextension/server.h"
#  include "webextensionadapter.h"
#endif

#include <QtQml/qqmlextensionplugin.h>

#include <QApplication>

Q_IMPORT_QML_PLUGIN(Mozilla_VPNPlugin);

namespace {
Logger logger("CommandUI");
}

CommandUI::CommandUI(QObject* parent) : Command(parent, "ui", "Start the UI.") {
  MZ_COUNT_CTOR(CommandUI);
}

CommandUI::~CommandUI() { MZ_COUNT_DTOR(CommandUI); }

int CommandUI::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option minimizedOption("m", "minimized",
                                            "Start minimized.");
  CommandLineParser::Option startAtBootOption("s", "start-at-boot",
                                              "Start at boot (if configured).");
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

  if (hOption.m_set) {
    clp.showHelp(this, appName, options, false, false);
    return 0;
  }

  // Change the application organization for testing mode.
  // This should ensure we wind up with a different settings file.
  if (testingOption.m_set) {
    QCoreApplication::setOrganizationName("Mozilla Testing");

    LogHandler::setStderr(true);
  }

  return runQmlApp([&]() {
    Telemetry::startTimeToFirstScreenTimer();

    if (testingOption.m_set) {
      Constants::setStaging();

      // Provide a mocked AppListProvider for testing.
      AppPermission::mock();

      // When running automated tests, create a mocked daemon.
      MockDaemon* daemon = new MockDaemon(qApp);
      qputenv("MVPN_CONTROL_SOCKET", daemon->socketPath().toLocal8Bit());
    }

    MozillaVPN vpn;
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
    if (EventListener::checkForInstances(
            I18nStrings::instance()->t(I18nStrings::ProductName))) {
      // If we are given URL parameters, send them to the UI socket and exit.
      for (const QString& value : tokens) {
        QUrl url(value);
        if (!url.isValid() || (url.scheme() != Constants::DEEP_LINK_SCHEME)) {
          logger.error() << "Invalid link:" << value;
        } else {
          EventListener::sendDeepLink(url);
        }
      }

      logger.debug() << "Terminating the current process";
      return 0;
    }

    // This class receives communications from other instances.
    EventListener eventListener;
#endif

#ifdef MZ_ANDROID
    // https://bugreports.qt.io/browse/QTBUG-82617
    // Currently there is a crash happening on exit with Huawei devices.
    // Until this is fixed, setting this variable is the "official" workaround.
    // We certainly should look at this once 6.6 is out.
#  if QT_VERSION >= 0x060800
#    error We have forgotten to remove this Huawei hack!
#  endif
    if (AndroidCommons::GetManufacturer() == "Huawei") {
      qputenv("QT_ANDROID_NO_EXIT_CALL", "1");
    }
#endif
    // This object _must_ live longer than MozillaVPN to avoid shutdown crashes.
    QQmlApplicationEngine* engine = new QQmlApplicationEngine();
    QmlEngineHolder engineHolder(engine);

    // TODO pending #3398
    QQmlContext* ctx = engine->rootContext();
    ctx->setContextProperty("QT_QUICK_BACKEND", qgetenv("QT_QUICK_BACKEND"));

    // Glean.rs
    QString gleanChannel = "production";
    if (testingOption.m_set) {
      gleanChannel = "testing";
    } else if (!Constants::inProduction()) {
      gleanChannel = "staging";
    }
    MZGlean::initialize(gleanChannel);
    // Clear leftover Glean.js stored data.
    // TODO: This code can be removed starting one year after it is released.
    auto offlineStorageDirectory =
        QDir(QmlEngineHolder::instance()->engine()->offlineStoragePath() +
             "/Databases");
    if (offlineStorageDirectory.exists()) {
      QStringList files = offlineStorageDirectory.entryList();
      for (const QString& file : files) {
        // Note: This is kinda dumb, it doesn't really know that this is
        // Glean.js' storage. Since Glean.js was the only thing using sqlite in
        // the app at the time of implementation this is fine. If we ever add
        // other SQLite using things, then we need to change this.
        if (file.endsWith(".sqlite")) {
          QFile::remove(offlineStorageDirectory.absoluteFilePath(file));
        }
      }
    }

    Lottie::initialize(engine, QString(NetworkManager::userAgent()));
    Nebula::Initialize(engine);
    I18nStrings::initialize();

    // Cleanup previous temporary files.
    TemporaryDir::cleanupAll();

    vpn.setStartMinimized(minimizedOption.m_set ||
                          (qgetenv("MVPN_MINIMIZED") == "1"));

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

#ifdef MZ_LINUX
    XdgStartAtBootWatcher startAtBootWatcher;
#endif

    // Prior to Qt 6.5, there was no default QML import path. We must set one.
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    engine->addImportPath("qrc:/");
    engine->addImportPath("qrc:/qt/qml");
#endif

    QQuickImageProvider* provider = ImageProviderFactory::create(qApp);
    if (provider) {
      engine->addImageProvider(QString("app"), provider);
    }

    qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0,
                                 "MZAccessibleNotification",
                                 AccessibleNotification::instance());

    // TODO: MZI18n should be moved to QmlEngineHolder but it requires extra
    // work for the generation of i18nstrings.h/cpp for the unit-test app.
    qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZI18n",
                                 I18nStrings::instance());

#if MZ_IOS && QT_VERSION < 0x060300
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, &vpn, &App::quit);
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
        []() { MozillaVPN::instance()->deactivate(true); },
        Qt::DirectConnection);

    QObject::connect(vpn.controller(), &Controller::readyToQuit, &vpn,
                     &App::quit, Qt::QueuedConnection);

    // Here is the main QML file.
    const QUrl url(QStringLiteral("qrc:/qt/qml/Mozilla/VPN/main.qml"));
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
#endif

#ifdef MVPN_WEBEXTENSION
    WebExtension::Server extensionServer(new WebExtensionAdapter(qApp));
    QObject::connect(vpn.controller(), &Controller::readyToQuit,
                     &extensionServer, &WebExtension::Server::close);
#endif

#ifdef MZ_ANDROID
    // If we are created with an url intent, auto pass that.
    QUrl maybeURL = AndroidVPNActivity::getOpenerURL();
    if (!maybeURL.isValid()) {
      logger.error() << "Error in deep-link:" << maybeURL.toString();
    } else {
      Navigator::instance()->requestDeepLink(url);
    }
    // Whenever the Client is re-opened with a new url
    // pass that to the navigator
    QObject::connect(
        AndroidVPNActivity::instance(), &AndroidVPNActivity::onOpenedWithUrl,
        [](QUrl url) { Navigator::instance()->requestDeepLink(url); });
#else
    // If there happen to be navigation URLs, send them to the navigator class.
    for (const QString& value : tokens) {
      QUrl url(value);
      if (!url.isValid() || (url.scheme() != Constants::DEEP_LINK_SCHEME)) {
        logger.error() << "Invalid link:" << value;
      } else {
        Navigator::instance()->requestDeepLink(url);
      }
    }
#endif

    KeyRegenerator keyRegenerator;
    // Let's go.
    return qApp->exec();
  });
}

static Command::RegistrationProxy<CommandUI> s_commandUI;
