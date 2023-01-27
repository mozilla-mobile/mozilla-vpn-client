/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingswatcher.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <QMetaObject>

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
  CONNECT(protectSelectedAppsChanged);
  CONNECT(dnsProviderFlagsChanged);
  CONNECT(userDNSChanged);

#undef CONNECT
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

  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    return;
  }

  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eSilentSwitch,
                               TaskControllerAction::eServerCoolDownNotNeeded));
}
