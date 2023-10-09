/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingswatcher.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <QMetaObject>

#include "connectionmanager.h"
#include "dnshelper.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "taskscheduler.h"

namespace {
Logger logger("SettingsWatcher");
}

SettingsWatcher::SettingsWatcher(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(SettingsWatcher);

  SettingsHolder* settingsHolder = SettingsHolder::instance();

#define CONNECT(x)                                  \
  connect(settingsHolder, &SettingsHolder::x, this, \
          &SettingsWatcher::maybeServerSwitch);

  CONNECT(captivePortalAlertChanged);
  CONNECT(vpnDisabledAppsChanged);

#undef CONNECT

#define DNS_CONNECT(x)                                                  \
  connect(settingsHolder, &SettingsHolder::x, this, [this]() {          \
    SettingsHolder* settingsHolder = SettingsHolder::instance();        \
    if (settingsHolder->dnsProviderFlags() != SettingsHolder::Custom || \
        DNSHelper::validateUserDNS(settingsHolder->userDNS())) {        \
      maybeServerSwitch();                                              \
    }                                                                   \
  });

  DNS_CONNECT(dnsProviderFlagsChanged);
  DNS_CONNECT(userDNSChanged);

#undef DNS_CONNECT

  connect(MozillaVPN::instance()->connectionManager(),
          &ConnectionManager::stateChanged, this, [this]() {
            // m_operationRunning is set to true when the Connection Manager is
            // in StateOn. So, if we see a change, it means that the new
            // settings values have been taken in consideration. We are ready to
            // schedule a new TaskControllerAction if needed.
            if (!MozillaVPN::instance()->connectionManager()->isVPNActive() &&
                m_operationRunning) {
              logger.debug() << "Resetting the operation running state";
              m_operationRunning = false;
            }
          });
}

SettingsWatcher::~SettingsWatcher() { MZ_COUNT_DTOR(SettingsWatcher); }

// static
SettingsWatcher* SettingsWatcher::instance() {
  static SettingsWatcher* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new SettingsWatcher(qApp);
  }
  return s_instance;
}

void SettingsWatcher::maybeServerSwitch() {
  logger.debug() << "Settings changed!";

  if (!MozillaVPN::instance()->connectionManager()->isVPNActive() ||
      m_operationRunning) {
    return;
  }

  m_operationRunning = true;

  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eSilentSwitch,
                               ConnectionManager::eServerCoolDownNotNeeded));
}

void SettingsWatcher::operationCompleted() { m_operationRunning = false; }
