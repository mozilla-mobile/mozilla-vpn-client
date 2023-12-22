/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keyregenerator.h"

#include "constants.h"
#include "controller.h"
#include "feature/feature.h"
#include "leakdetector.h"
#include "logger.h"
#include "mfbt/checkedint.h"
#include "models/device.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/account/taskaccount.h"
#include "tasks/adddevice/taskadddevice.h"
#include "taskscheduler.h"

namespace {
Logger logger("KeyRegenerator");
}

KeyRegenerator::KeyRegenerator() {
  MZ_COUNT_CTOR(KeyRegenerator);

  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn, &MozillaVPN::stateChanged, this, &KeyRegenerator::stateChanged);
  connect(vpn->controller(), &Controller::stateChanged, this,
          &KeyRegenerator::stateChanged);
  connect(&m_timer, &QTimer::timeout, this, &KeyRegenerator::stateChanged);
  connect(Feature::get(Feature::Feature_keyRegeneration),
          &Feature::supportedChanged, this, &KeyRegenerator::stateChanged);

  stateChanged();
}

KeyRegenerator::~KeyRegenerator() { MZ_COUNT_DTOR(KeyRegenerator); }

void KeyRegenerator::stateChanged() {
  logger.debug() << "Let's check if the key has to be regenerated";

  m_timer.stop();

  if (!Feature::get(Feature::Feature_keyRegeneration)->isSupported()) {
    logger.debug() << "Feature disabled";
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->state() != App::StateMain ||
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

    CheckedInt<int> value(static_cast<int>(diff));
    value *= 1000;

    m_timer.start(value.isValid() ? value.value()
                                  : std::numeric_limits<int>::max());
    return;
  }

  logger.debug() << "Triggering the key regeneration";

  TaskScheduler::scheduleTask(
      new TaskAddDevice(Device::currentDeviceName(), Device::uniqueDeviceId()));
  TaskScheduler::scheduleTask(new TaskAccount(ErrorHandler::PropagateError));

  CheckedInt<int> value(Constants::keyRegeneratorTimeSec());
  value *= 1000;

  m_timer.start(value.isValid() ? value.value()
                                : std::numeric_limits<int>::max());
}
