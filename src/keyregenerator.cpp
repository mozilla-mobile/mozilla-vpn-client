/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keyregenerator.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/account/taskaccount.h"
#include "tasks/adddevice/taskadddevice.h"
#include "taskscheduler.h"

namespace {
Logger logger(LOG_MAIN, "KeyRegenerator");
}

KeyRegenerator::KeyRegenerator() {
  MVPN_COUNT_CTOR(KeyRegenerator);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  connect(vpn, &MozillaVPN::stateChanged, this, &KeyRegenerator::stateChanged);
  connect(vpn->controller(), &Controller::stateChanged, this,
          &KeyRegenerator::stateChanged);
  connect(&m_timer, &QTimer::timeout, this, &KeyRegenerator::stateChanged);
  connect(Feature::get(Feature::Feature_keyRegeneration),
          &Feature::supportedChanged, this, &KeyRegenerator::stateChanged);

  stateChanged();
}

KeyRegenerator::~KeyRegenerator() { MVPN_COUNT_DTOR(KeyRegenerator); }

void KeyRegenerator::stateChanged() {
  logger.debug() << "Let's check if the key has to be regenerated";

  m_timer.stop();

  if (!Feature::get(Feature::Feature_keyRegeneration)->isSupported()) {
    logger.debug() << "Feature disabled";
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  if (vpn->state() != MozillaVPN::StateMain ||
      vpn->controller()->state() != Controller::StateOff) {
    logger.debug() << "Wrong state";
    return;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  // Let's support migration to this new key value.
  if (!settingsHolder->hasKeyRegenerationTimeSec()) {
    logger.debug() << "key regeneration time set";
    settingsHolder->setKeyRegenerationTimeSec(
        QDateTime::currentSecsSinceEpoch());
  }

  qint64 diff = Constants::keyRegeneratorTimeSec() -
                (QDateTime::currentSecsSinceEpoch() -
                 settingsHolder->keyRegenerationTimeSec());
  if (diff > 0) {
    logger.debug() << "Key regeneration in" << diff << "secs";
    m_timer.start(diff * 1000);
    return;
  }

  logger.debug() << "Triggering the key regeneration";

  TaskScheduler::scheduleTask(
      new TaskAddDevice(Device::currentDeviceName(), Device::uniqueDeviceId()));
  TaskScheduler::scheduleTask(new TaskAccount());

  m_timer.start(Constants::keyRegeneratorTimeSec() * 1000);
}
