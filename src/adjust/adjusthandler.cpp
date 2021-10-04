/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusthandler.h"
#include "adjustproxy.h"
#include "constants.h"
#include "mozillavpn.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosadjusthelper.h"
#endif
#ifdef MVPN_ANDROID
#  include <QAndroidJniObject>
#endif

#include <QString>
#include <QRandomGenerator>

namespace {
bool s_initialized = false;
}  // namespace

void AdjustHandler::maybeInitialize() {
  if (s_initialized) {
    return;
  }

  s_initialized = true;

  AdjustProxy* adjustProxy = new AdjustProxy(MozillaVPN::instance());
  QObject::connect(MozillaVPN::instance()->controller(),
                   &Controller::readyToQuit, adjustProxy, &AdjustProxy::close);
  for (int i = 0; i < 5; i++) {
    quint16 port = QRandomGenerator::global()->bounded(1024, 65536);
    bool succeeded = adjustProxy->initialize(port);
    if (succeeded) {
      break;
    }
  }

  if (!adjustProxy->isListening()) {
    return;
  }

#ifdef MVPN_ANDROID
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "onVpnInit", "(ZI)V",
      Constants::inProduction(), adjustProxy->serverPort());
#endif

#ifdef MVPN_IOS
  IOSAdjustHelper::initialize(adjustProxy->serverPort());
#endif
}

void AdjustHandler::trackEvent(const QString& event) {
  if (!AdjustProxy::instance()->isListening()) {
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
