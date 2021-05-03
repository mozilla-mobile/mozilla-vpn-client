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
  if (vpn->state() != MozillaVPN::StateMain &&
      // The Disconnected notification should be triggerable
      // on StateInitialize, in case the user was connected during a log-out
      // Otherwise existing notifications showing "connected" would update
      !(vpn->state() == MozillaVPN::StateInitialize &&
        vpn->controller()->state() == Controller::StateOff)) {
    return;
  }

  QString title;
  QString message;

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      m_connected = true;

      if (m_switching) {
        m_switching = false;

        //% "VPN Switched Servers"
        title = qtTrId("vpn.systray.statusSwitch.title");
        //% "Switched from %1, %2 to %3, %4"
        //: Shown as message body in a notification. %1 and %3 are countries, %2
        //: and %4 are cities.
        message = qtTrId("vpn.systray.statusSwtich.message")
                      .arg(m_switchingServerCountry)
                      .arg(m_switchingServerCity)
                      .arg(vpn->currentServer()->countryName())
                      .arg(vpn->currentServer()->cityName());
      } else {
        //% "VPN Connected"
        title = qtTrId("vpn.systray.statusConnected.title");
        //% "Connected to %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is
        //: the city.
        message = qtTrId("vpn.systray.statusConnected.message")
                      .arg(vpn->currentServer()->countryName())
                      .arg(vpn->currentServer()->cityName());
      }
      break;

    case Controller::StateOff:
      if (m_connected) {
        m_connected = false;

        //% "VPN Disconnected"
        title = qtTrId("vpn.systray.statusDisconnected.title");
        //% "Disconnected from %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is
        //: the city.
        message = qtTrId("vpn.systray.statusDisconnected.message")
                      .arg(vpn->currentServer()->countryName())
                      .arg(vpn->currentServer()->cityName());
      }
      break;

    case Controller::StateSwitching:
      m_connected = true;

      m_switching = true;
      m_switchingServerCountry = vpn->currentServer()->countryName();
      m_switchingServerCity = vpn->currentServer()->cityName();
      break;

    default:
      break;
  }

  Q_ASSERT(title.isEmpty() == message.isEmpty());

  if (!title.isEmpty()) {
    notify(title, message, 2);
  }
}
