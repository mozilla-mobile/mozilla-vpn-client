/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusthandler.h"
#include "constants.h"
#include "logger.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosadjusthelper.h"
#endif
#ifdef MVPN_ANDROID
#  include <QAndroidJniObject>
#endif

#include <QString>

namespace {
Logger logger(LOG_MAIN, "AdjustHandler");
}  // namespace

const QString AdjustHandler::eventToToken(AdjustEvent event) {
  switch(event) {
    case AdjustEvent::SubscriptionCompleted:
#ifdef MVPN_IOS
      return "jl72xm";
#endif
#ifdef MVPN_ANDROID
      return "o1mn9m";
#endif
    default:
      logger.error() << "Unknown Adjust event provided: " << event;
      return "";
  }
}

void AdjustHandler::initialize() {
#ifdef MVPN_ANDROID
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "onVpnInit", "(Z)V",
      Constants::inProduction());
#endif

#ifdef MVPN_IOS
  IOSAdjustHelper::initialize();
#endif
}

void AdjustHandler::trackEvent(AdjustEvent event) {
  const QString eventToken = AdjustHandler::eventToToken(event);

#ifdef MVPN_ANDROID
  QAndroidJniObject javaMessage = QAndroidJniObject::fromString(eventToken);
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNApplication", "trackEvent",
      "(Ljava/lang/String;)V", javaMessage.object<jstring>());
#endif

#ifdef MVPN_IOS
  IOSAdjustHelper::trackEvent(eventToken);
#endif
}