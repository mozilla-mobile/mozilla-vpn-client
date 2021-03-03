/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "notificationhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#if defined(MVPN_IOS)
#  include "platforms/ios/iosnotificationhandler.h"
#elif defined(MVPN_ANDROID)
#  include "platforms/android/androidnotificationhandler.h"
#else
#  include "systemtraynotificationhandler.h"
#endif

namespace {
Logger logger(LOG_MAIN, "NotificationHandler");
}

// static
NotificationHandler* NotificationHandler::create(QObject* parent) {
#if defined(MVPN_IOS)
  return new IOSNotificationHandler(parent);
#elif defined(MVPN_ANDROID)
  return new AndroidNotificationHandler(parent);
#else
  return new SystemTrayNotificationHandler(parent);
#endif
}

NotificationHandler::NotificationHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(NotificationHandler);
}

NotificationHandler::~NotificationHandler() {
  MVPN_COUNT_DTOR(NotificationHandler);
}

void NotificationHandler::showNotification() {
  logger.log() << "Show notification";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() != MozillaVPN::StateMain) {
    return;
  }

  QString title;
  QString message;

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      m_connected = true;

      if (m_switching) {
        m_switching = false;

        title = qtTrId("vpn.systray.statusSwitch.title");
        message = qtTrId("vpn.systray.statusSwtich.message")
                      .arg(m_switchingServerCountry)
                      .arg(m_switchingServerCity)
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city());
      } else {

        title = qtTrId("vpn.systray.statusConnected.title");
        message = qtTrId("vpn.systray.statusConnected.message")
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city());
      }
      break;

    case Controller::StateOff:
      if (m_connected) {
        m_connected = false;


        title = qtTrId("vpn.systray.statusDisconnected.title");
        message = qtTrId("vpn.systray.statusDisconnected.message")
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city());
      }
      break;

    case Controller::StateSwitching:
      m_connected = true;

      m_switching = true;
      m_switchingServerCountry = vpn->currentServer()->country();
      m_switchingServerCity = vpn->currentServer()->city();
      break;

    default:
      break;
  }

  Q_ASSERT(title.isEmpty() == message.isEmpty());

  if (!title.isEmpty()) {
    notify(title, message, 2);
  }
}
