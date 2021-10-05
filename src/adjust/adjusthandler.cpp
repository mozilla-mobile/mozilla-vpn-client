/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusthandler.h"
#include "adjustproxy.h"
#include "constants.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosadjusthelper.h"
#endif
#ifdef MVPN_ANDROID
#  include <QtAndroid>
#  include <QAndroidJniObject>
#endif

#include <QString>
#include <QRandomGenerator>

namespace {
Logger logger(LOG_ADJUST, "AdjustHandler");
bool s_initialized = false;
AdjustProxy* s_adjustProxy = nullptr;
}  // namespace

void AdjustHandler::initialize() {
  if (s_initialized) {
    return;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  // If the user has not seen the telemetry policy view yet, we must wait.
  if (!settingsHolder->telemetryPolicyShown()) {
    QObject::connect(settingsHolder,
                     &SettingsHolder::telemetryPolicyShownChanged,
                     AdjustHandler::initialize);
    return;
  }

  s_initialized = true;

  if (!settingsHolder->gleanEnabled()) {
    // The user doesn't want to be tracked. Good!
    return;
  }

  QObject::connect(settingsHolder, &SettingsHolder::gleanEnabledChanged,
                   [](const bool& gleanEnabled) {
                     if (!gleanEnabled) {
                       forget();
                       // Let's keep the proxy on. Maybe Adjust needs to send
                       // requests to remove data on their servers.
                       return;
                     }
                   });

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  s_adjustProxy = new AdjustProxy(vpn);
  QObject::connect(vpn->controller(), &Controller::readyToQuit, s_adjustProxy,
                   &AdjustProxy::close);
  QObject::connect(adjustProxy, &AdjustProxy::acceptError,
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

#ifdef MVPN_ANDROID
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "onVpnInit", "(ZI)V",
      Constants::inProduction(), s_adjustProxy->serverPort());
#endif

#ifdef MVPN_IOS
  IOSAdjustHelper::initialize(s_adjustProxy->serverPort());
#endif
}

void AdjustHandler::trackEvent(const QString& event) {
  if (!s_adjustProxy || !s_adjustProxy->isListening()) {
    logger.error() << "Adjust Proxy not listening; event tracking failed";
    return;
  }

#ifdef MVPN_ANDROID
  QAndroidJniObject javaMessage = QAndroidJniObject::fromString(event);
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "trackEvent",
      "(Ljava/lang/String;)V", javaMessage.object<jstring>());
#endif

#ifdef MVPN_IOS
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

#ifdef MVPN_ANDROID
  QAndroidJniObject activity = QtAndroid::androidActivity();
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "forget",
      "(Landroid/app/Activity;)V", activity.object());
#endif

#ifdef MVPN_IOS
  IOSAdjustHelper::forget();
#endif
}
