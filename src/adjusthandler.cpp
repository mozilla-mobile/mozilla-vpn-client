/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusthandler.h"
#include "adjustproxy.h"
#include "constants.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosadjusthelper.h"
#endif
#ifdef MVPN_ANDROID
#  include <QAndroidJniObject>
#endif

#include <QString>

namespace {
bool s_initialized = false;
}  // namespace

void AdjustHandler::maybeInitialize(quint16 proxyPort) {
  if (s_initialized) {
    return;
  }

  if (!AdjustProxy::instance()->isListening()) {
    return;
  }

#ifdef MVPN_ANDROID
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "onVpnInit", "(ZI)V",
      Constants::inProduction(), proxyPort);
#endif

#ifdef MVPN_IOS
  IOSAdjustHelper::initialize(proxyPort);
#endif

  s_initialized = true;
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
