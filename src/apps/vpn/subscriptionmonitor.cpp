/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptionmonitor.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <QMetaObject>

#include "dnshelper.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "taskscheduler.h"

namespace {
Logger logger("SubscriptionMonitor");
}

/**
     * Finally, any singleton should define some business logic, which can be
     * executed on its instance.
     * link: https://refactoring.guru/design-patterns/singleton/cpp/example#example-1
*/

SubscriptionMonitor::SubscriptionMonitor(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(SubscriptionMonitor);

//   SettingsHolder* settingsHolder = SettingsHolder::instance();

// #define CONNECT(x)                                  \
//   connect(settingsHolder, &SettingsHolder::x, this, \
//           &SubscriptionMonitor::maybeServerSwitch);

//   CONNECT(captivePortalAlertChanged);
//   CONNECT(protectSelectedAppsChanged);
//   CONNECT(vpnDisabledAppsChanged);

// #undef CONNECT

// #define DNS_CONNECT(x)                                                  \
//   connect(settingsHolder, &SettingsHolder::x, this, [this]() {          \
//     SettingsHolder* settingsHolder = SettingsHolder::instance();        \
//     if (settingsHolder->dnsProviderFlags() != SettingsHolder::Custom || \
//         DNSHelper::validateUserDNS(settingsHolder->userDNS())) {        \
//       maybeServerSwitch();                                              \
//     }                                                                   \
//   });

//   DNS_CONNECT(dnsProviderFlagsChanged);
//   DNS_CONNECT(userDNSChanged);

// #undef DNS_CONNECT

//   connect(MozillaVPN::instance()->controller(), &Controller::stateChanged, this,
//           [this]() {
//             Controller::State state =
//                 MozillaVPN::instance()->controller()->state();
//             // m_operationRunning is set to true when the Controller is in
//             // StateOn. So, if we see a change, it means that the new settings
//             // values have been taken in consideration. We are ready to
//             // schedule a new TaskControllerAction if needed.
//             if (state != Controller::StateOn && state != Controller::StateOff &&
//                 m_operationRunning) {
//               logger.debug() << "Resetting the operation running state";
//               m_operationRunning = false;
//             }
//           });
}

SubscriptionMonitor::~SubscriptionMonitor() { MZ_COUNT_DTOR(SubscriptionMonitor); }

// static
SubscriptionMonitor* SubscriptionMonitor::instance() {
  static SubscriptionMonitor* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new SubscriptionMonitor(qApp);
  }
  return s_instance;
}

// void SubscriptionMonitor::maybeServerSwitch() {
//   logger.debug() << "Settings changed!";

//   if (MozillaVPN::instance()->controller()->state() != Controller::StateOn ||
//       m_operationRunning) {
//     return;
//   }

//   m_operationRunning = true;

//   TaskScheduler::deleteTasks();
//   TaskScheduler::scheduleTask(
//       new TaskControllerAction(TaskControllerAction::eSilentSwitch,
//                                TaskControllerAction::eServerCoolDownNotNeeded));
// }

void SubscriptionMonitor::operationCompleted() { m_operationRunning = false; }
