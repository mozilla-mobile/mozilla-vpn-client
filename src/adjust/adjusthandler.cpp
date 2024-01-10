/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusthandler.h"

#include "adjustproxy.h"
#include "app.h"
#include "context/constants.h"
#include "logging/logger.h"
#include "settingsholder.h"

#ifdef MZ_IOS
#  include "platforms/ios/iosadjusthelper.h"
#endif
#ifdef MZ_ANDROID
#  include <QJniObject>

#  include "platforms/android/androidcommons.h"
#endif

#include <QCoreApplication>
#include <QRandomGenerator>
#include <QString>

namespace {
Logger logger("AdjustHandler");
bool s_initialized = false;
AdjustProxy* s_adjustProxy = nullptr;
constexpr auto CORE_APPLICATION =
    "org/mozilla/firefox/qt/common/CoreApplication";
}  // namespace

void AdjustHandler::initialize() {
  if (s_initialized) {
    return;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

// This file should only run if MZ_ADJUST is active, which means we shouldn't
// need to check for the flag here. However, to prevent compiler errors we need
// to wrap every use of the adjustActivatable setting in this flag.
#ifdef MZ_ADJUST
  if (settingsHolder->firstExecution() &&
      !settingsHolder->hasAdjustActivatable()) {
    // We want to activate Adjust only for new users.
    logger.debug() << "First execution detected. Let's make adjust activatable";
    settingsHolder->setAdjustActivatable(true);
  }
#endif

  App* app = App::instance();

  // If the app has not started yet, let's wait.
  if (app->state() == App::StateInitialize) {
    QObject::connect(app, &App::stateChanged, AdjustHandler::initialize);
    return;
  }

  s_initialized = true;

  if (!settingsHolder->gleanEnabled()) {
    // The user doesn't want to be tracked. Good!
    logger.debug() << "Telemetry policy disabled. Bail out";
    return;
  }

#ifdef MZ_ADJUST
  if (!settingsHolder->adjustActivatable()) {
    // This is a pre-adjustSDK user. We don't want to activate the tracking.
    logger.debug() << "Adjust is not activatable. Bail out";
    return;
  }
#endif

  QObject::connect(settingsHolder, &SettingsHolder::gleanEnabledChanged, []() {
    if (!SettingsHolder::instance()->gleanEnabled()) {
      forget();
      // Let's keep the proxy on. Maybe Adjust needs to send
      // requests to remove data on their servers.
    }
  });

  s_adjustProxy = new AdjustProxy(app);
  QObject::connect(qApp, &QCoreApplication::aboutToQuit, s_adjustProxy,
                   &AdjustProxy::close);
  QObject::connect(s_adjustProxy, &AdjustProxy::acceptError,
                   [](QAbstractSocket::SocketError socketError) {
                     logger.error()
                         << "Adjust Proxy connection error: " << socketError;
                   });
  for (int i = 0; i < 5; i++) {
    quint16 port = QRandomGenerator::global()->bounded(1024, 65536);
    bool succeeded = s_adjustProxy->initialize(port);
    if (succeeded) {
      break;
    }
  }

  if (!s_adjustProxy->isListening()) {
    logger.error() << "Adjust Proxy listening failed";
    return;
  }

#ifdef MZ_ANDROID
  QJniObject::callStaticMethod<void>(CORE_APPLICATION, "initializeAdjust",
                                     "(ZI)V", Constants::inProduction(),
                                     s_adjustProxy->serverPort());
#endif

#ifdef MZ_IOS
  IOSAdjustHelper::initialize(s_adjustProxy->serverPort());
#endif
}

void AdjustHandler::trackEvent(const QString& event) {
  if (!s_adjustProxy || !s_adjustProxy->isListening()) {
    logger.error() << "Adjust Proxy not listening; event tracking failed";
    return;
  }

#ifdef MZ_ADJUST
  if (!SettingsHolder::instance()->adjustActivatable()) {
    logger.debug() << "Adjust is not activatable. Bail out";
    return;
  }
#endif

#ifdef MZ_ANDROID
  QJniObject javaMessage = QJniObject::fromString(event);
  QJniObject::callStaticMethod<void>(CORE_APPLICATION, "trackEvent",
                                     "(Ljava/lang/String;)V",
                                     javaMessage.object<jstring>());
#endif

#ifdef MZ_IOS
  IOSAdjustHelper::trackEvent(event);
#endif
}

void AdjustHandler::forget() {
  logger.debug() << "Adjust Proxy forget";

  if (!s_adjustProxy || !s_adjustProxy->isListening()) {
    logger.error()
        << "Adjust Proxy cannot forget because proxy is not listening!";
    return;
  }

#ifdef MZ_ANDROID
  QJniObject activity = AndroidCommons::getActivity();
  QJniObject::callStaticMethod<void>(CORE_APPLICATION, "forget",
                                     "(Landroid/app/Activity;)V",
                                     activity.object());
#endif

#ifdef MZ_IOS
  IOSAdjustHelper::forget();
#endif
}
