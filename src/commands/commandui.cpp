/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandui.h"

#include <lottie.h>
#include <nebula.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWindow>

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
#include "networkmanager.h"
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
#endif

#ifdef MZ_MACOS
#  include "platforms/macos/macosmenubar.h"
#  include "platforms/macos/macosutils.h"
#endif

#ifdef MZ_ANDROID
#  include <QQuickWindow>
#  include <QSGRendererInterface>

#  include "platforms/android/androidcommons.h"
#  include "platforms/android/androidutils.h"
#  include "platforms/android/androidvpnactivity.h"
#endif

#ifdef MVPN_WEBEXTENSION
#  include "webextension/server.h"
#  include "webextensionadapter.h"
#endif

#ifdef MZ_WINDOWS
#  include <windows.h>

#  include <QQuickWindow>
#  include <QSGRendererInterface>

#  include "eventlistener.h"
#  include "platforms/windows/windowscommons.h"
#  include "platforms/windows/windowsutils.h"
#  include "theme.h"
#endif

#ifdef MZ_WASM
#  include "platforms/wasm/wasmwindowcontroller.h"
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

    LogHandler::instance()->setStderr(true);
  }

  // If there is another instance, the execution terminates here.
#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
  if (EventListener::checkForInstances()) {
    QTextStream stream(stderr);
    stream << "Existing instance found" << Qt::endl;

    // If we are given URL parameters, send them to the UI socket and exit.
    for (const QString& value : tokens) {
      QUrl url(value);
      if (!url.isValid() || (url.scheme() != Constants::DEEP_LINK_SCHEME)) {
        stream << "Invalid link:" << value << Qt::endl;
      } else {
        stream << "Sending link" << Qt::endl;
        EventListener::sendDeepLink(url);
      }
    }

    return 0;
  }
  // This class receives communications from other instances.
  EventListener eventListener;
#endif

#ifdef MZ_ANDROID
  // Configure graphics rendering for Android
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::Round);
  if (AndroidUtils::isChromeOSContext()) {
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
  }
#endif

#ifdef MZ_WINDOWS
  // Configure graphics rendering for Windows
  SetProcessDPIAware();
  if (WindowsCommons::requireSoftwareRendering()) {
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
  }
#endif

#ifdef MVPN_WEBEXTENSION
  std::unique_ptr<WebExtension::Server> extensionServer{nullptr};
#endif
  std::unique_ptr<KeyRegenerator> keyRegenerator{nullptr};

  // Ensure that external styling hints are disabled.
  qunsetenv("QT_STYLE_OVERRIDE");
  return MozillaVPN::runGuiApp([&]() {
    // Already intiziaized: VPN & qAPP.
    // Note: this lambda will exit, so don't use it's scope for long-living
    // objects.
    auto const vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);
    Telemetry::startTimeToFirstScreenTimer();

    if (testingOption.m_set) {
      Constants::setStaging();

      // Provide a mocked AppListProvider for testing.
      AppPermission::mock();

      // When running automated tests, create a mocked daemon.
      MockDaemon* daemon = new MockDaemon(qApp);
      qputenv("MVPN_CONTROL_SOCKET", daemon->socketPath().toLocal8Bit());
    }
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

#ifdef MZ_ANDROID
    // https://bugreports.qt.io/browse/QTBUG-82617
    // Currently there is a crash happening on exit with Huawei devices.
    // Until this is fixed, setting this variable is the "official" workaround.
    // We certainly should look at this once 6.6 is out.
#  if QT_VERSION >= 0x061000
#    error We have forgotten to remove this Huawei hack!
#  endif
    if (AndroidCommons::GetManufacturer() == "Huawei") {
      qputenv("QT_ANDROID_NO_EXIT_CALL", "1");
    }
#endif
    auto const engineHolder = QmlEngineHolder::instance();
    auto const engine =
        static_cast<QQmlApplicationEngine*>(engineHolder->engine());
    Q_ASSERT(engine);
    // // Glean.rs
    QString gleanChannel = "production";
    if (testingOption.m_set) {
      gleanChannel = "testing";
    } else if (!Constants::inProduction()) {
      gleanChannel = "staging";
    }
    MZGlean::initialize(gleanChannel);
    Lottie::initialize(engine, QString(NetworkManager::userAgent()));
    Nebula::Initialize(engine);

    // Cleanup previous temporary files.
    TemporaryDir::cleanupAll();

    vpn->setStartMinimized(minimizedOption.m_set ||
                           (qgetenv("MVPN_MINIMIZED") == "1"));

    // Font loader
    FontLoader::loadFonts();

    vpn->initialize();
    logger.debug() << "VPN initialized";

    // Prior to Qt 6.5, there was no default QML import path. We must set one.
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    engine->addImportPath("qrc:/");
    engine->addImportPath("qrc:/qt/qml");
    logger.debug() << "Added QML import paths";
#endif
    QQuickImageProvider* provider = ImageProviderFactory::create(qApp);
    if (provider) {
      engine->addImageProvider(QString("app"), provider);
    }
    logger.debug() << "Added QML image provider";

    qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0,
                                 "MZAccessibleNotification",
                                 AccessibleNotification::instance());

    // TODO: MZI18n should be moved to QmlEngineHolder but it requires extra
    // work for the generation of i18nstrings.h/cpp for the unit-test app.
    qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZI18n",
                                 I18nStrings::instance());
    logger.debug() << "Registered I18nStrings";

#if MZ_IOS && QT_VERSION < 0x060300
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, vpn, &App::quit);
#else
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, vpn, [] {
      // Submit the main ping one last time.
      mozilla::glean_pings::Main.submit();
      // During shutdown Glean will attempt to finish all tasks
      // and submit all enqueued pings (including the one we
      // just sent).
      MZGlean::shutdown();

      MozillaVPN::instance()->aboutToQuit();
    });
#endif

    QObject::connect(
        qApp, &QGuiApplication::commitDataRequest, vpn,
        []() { MozillaVPN::instance()->deactivate(true); },
        Qt::DirectConnection);

    QObject::connect(vpn->controller(), &Controller::readyToQuit, vpn,
                     &App::quit, Qt::QueuedConnection);

#ifdef MZ_ANDROID
    // On Android we need to make sure when we load a QML application that the
    // context is set. Qt ___should___ do this for us, but it seems that in some
    // cases it does not. So we chant dark JNI magic to make sure the
    // VPNActivity is set as context provider.
    logger.debug() << "Forcing activity publish";
    AndroidCommons::forcePublishActivity();
    // In case we have a pending exception, clear it before loading main.qml
    AndroidCommons::clearPendingJavaException("before main.qml load");
#endif
    const QUrl url(QStringLiteral("qrc:/qt/qml/Mozilla/VPN/main.qml"));
    logger.debug() << "Loading main QML file:" << url.toString();
    engine->load(url);
    logger.debug() << "Loaded main QML file";
    if (!engineHolder->hasWindow()) {
      logger.error() << "Failed to load " << url.toString();
    }

#ifdef MZ_WINDOWS
    auto const updateWindowDecoration = [&engineHolder]() {
      auto const window = engineHolder->window();
      WindowsUtils::updateTitleBarColor(window,
                                        Theme::instance()->isThemeDark());
      WindowsUtils::setDockIcon(window, QImage(":/ui/resources/logo-dock.png"));
      WindowsUtils::setTitleBarIcon(window,
                                    Theme::instance()->getTitleBarIcon());
    };
    QObject::connect(Theme::instance(), &Theme::changed,
                     updateWindowDecoration);
    updateWindowDecoration();
#endif

#ifdef MZ_MACOS
    MacOSMenuBar menuBar;
    menuBar.initialize();

    QObject::connect(vpn, &MozillaVPN::stateChanged, &menuBar,
                     &MacOSMenuBar::controllerStateChanged);

    QObject::connect(vpn->controller(), &Controller::stateChanged, &menuBar,
                     &MacOSMenuBar::controllerStateChanged);

#endif
    NotificationHandler* notificationHandler =
        NotificationHandler::create(qApp);
    QObject::connect(vpn->controller(), &Controller::stateChanged,
                     notificationHandler,
                     &NotificationHandler::showNotification);

    QObject::connect(
        SettingsHolder::instance(), &SettingsHolder::languageCodeChanged, []() {
          logger.debug() << "Retranslating";
          QmlEngineHolder::instance()->engine()->retranslate();
          NotificationHandler::instance()->retranslate();
          I18nStrings::instance()->retranslate();
          AddonManager::instance()->retranslate();

#ifdef MZ_MACOS
          if (auto* menuBar = MacOSMenuBar::instance()) {
            menuBar->retranslate();
          }
#endif

#ifdef MZ_WASM
          WasmWindowController::instance()->retranslate();
#endif

          MozillaVPN::instance()->serverCountryModel()->retranslate();
          MozillaVPN::instance()->serverData()->retranslate();
        });

    InspectorHandler::initialize();
    logger.debug() << "Inspector Handler initialized";
#ifdef MZ_WASM
    WasmWindowController wasmWindowController;
#endif

#ifdef MVPN_WEBEXTENSION
    extensionServer.reset(
        new WebExtension::Server{new WebExtensionAdapter(qApp)});
    QObject::connect(vpn->controller(), &Controller::readyToQuit,
                     extensionServer.get(), &WebExtension::Server::close);
#endif

#ifdef MZ_ANDROID
    // If we are created with an url intent, auto pass that.
    QUrl maybeURL = AndroidVPNActivity::getOpenerURL();
    if (!maybeURL.isValid()) {
      logger.error() << "Error in deep-link:" << maybeURL.toString();
    } else {
      vpn->handleDeepLink(maybeURL);
    }
    // Whenever the client is re-opened with a new url pass it to the handler.
    connect(AndroidVPNActivity::instance(),
            &AndroidVPNActivity::onOpenedWithUrl, vpn,
            &MozillaVPN::handleDeepLink);
#else
    // If there happen to be navigation URLs, handle them.
    for (const QString& value : tokens) {
      QUrl url(value);
      if (!url.isValid() || (url.scheme() != Constants::DEEP_LINK_SCHEME)) {
        logger.error() << "Invalid link:" << value;
      } else {
        vpn->handleDeepLink(url);
      }
    }
#endif
    keyRegenerator.reset(new KeyRegenerator{});
    // We're ready to continue!
    logger.debug() << "CommandUI finished";
    return 0;
  });
}

static Command::RegistrationProxy<CommandUI> s_commandUI;
