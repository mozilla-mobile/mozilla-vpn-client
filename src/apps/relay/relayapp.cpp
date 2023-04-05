/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "relayapp.h"

#include <QCoreApplication>
#include <QQmlApplicationEngine>

#include "fontloader.h"
#include "frontend/navigationbarmodel.h"
#include "glean/mzglean.h"
#include "i18nstrings.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "lottie.h"
#include "nebula.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "temporarydir.h"
#include "theme.h"

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

namespace {
RelayApp* s_instance = nullptr;

Logger logger("RelayApp");
}  // namespace

// static
App* App::instance() { return RelayApp::instance(); }

RelayApp::RelayApp() : App(nullptr) {
  MZ_COUNT_CTOR(RelayApp);

  Q_ASSERT(!s_instance);
  s_instance = this;

  QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, &App::quit);

  QObject::connect(SettingsHolder::instance(),
                   &SettingsHolder::languageCodeChanged, []() {
                     logger.debug() << "Retranslating";
                     QmlEngineHolder::instance()->engine()->retranslate();
                     I18nStrings::instance()->retranslate();
                   });
}

RelayApp::~RelayApp() {
  MZ_COUNT_DTOR(RelayApp);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
RelayApp* RelayApp::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

bool RelayApp::initialize() {
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
  if (AndroidCommons::GetManufacturer() == "Huawei") {
    qputenv("QT_ANDROID_NO_EXIT_CALL", "1");
  }
#endif

  QQmlApplicationEngine* engine = new QQmlApplicationEngine();
  QmlEngineHolder engineHolder(engine);

  MZGlean::initialize();

  Lottie::initialize(engine, QString(NetworkManager::userAgent()));
  Nebula::Initialize(engine);
  I18nStrings::initialize();

  // Cleanup previous temporary files.
  TemporaryDir::cleanupAll();

  // Font loader
  FontLoader::loadFonts();

  qmlRegisterSingletonInstance("Mozilla.Relay", 1, 0, "Relay", this);

  // TODO: MZI18n should be moved to QmlEngineHolder but it requires extra
  // work for the generation of i18nstrings.h/cpp for the unit-test app.
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZI18n",
                               I18nStrings::instance());

  // Here is the main QML file.
  const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
  engine->load(url);
  if (!engineHolder.hasWindow()) {
    logger.error() << "Failed to load " << url.toString();
    return false;
  }

  InspectorHandler::initialize();

  registerNavigationBarButtons();

  return true;
}

// static
void RelayApp::registerNavigationBarButtons() {
  NavigationBarModel* nbm = NavigationBarModel::instance();
  nbm->appendButton(new NavigationBarButton(
      nbm, "navButton-home", "NavBarHomeTab", RelayApp::ScreenHome,
      "qrc:/nebula/resources/navbar/home.svg",
      "qrc:/nebula/resources/navbar/home-selected.svg"));

  NavigationBarButton* messageButton = new NavigationBarButton(
      nbm, "navButton-messages", "NavBarMessagesTab", RelayApp::ScreenMessaging,
      "qrc:/nebula/resources/navbar/messages.svg",
      "qrc:/nebula/resources/navbar/messages-selected.svg",
      "qrc:/nebula/resources/navbar/messages-notification.svg",
      "qrc:/nebula/resources/navbar/messages-notification-selected.svg");
  nbm->appendButton(messageButton);

  nbm->appendButton(new NavigationBarButton(
      nbm, "navButton-call", "NavBarCallTab", RelayApp::ScreenCall,
      "qrc:/nebula/resources/navbar/home.svg",
      "qrc:/nebula/resources/navbar/home-selected.svg"));

  nbm->appendButton(new NavigationBarButton(
      nbm, "navButton-settings", "NavBarSettingsTab", RelayApp::ScreenSettings,
      "qrc:/nebula/resources/navbar/settings.svg",
      "qrc:/nebula/resources/navbar/settings-selected.svg"));

  // TODO: call setHasNotification in messageButton
}
