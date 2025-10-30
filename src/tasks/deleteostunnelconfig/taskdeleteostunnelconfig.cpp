/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskdeleteostunnelconfig.h"

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

namespace {
Logger logger("TaskDeleteOSTunnelConfig");
}

TaskDeleteOSTunnelConfig::TaskDeleteOSTunnelConfig()
    : Task("TaskDeleteOSTunnelConfig") {
  MZ_COUNT_CTOR(TaskDeleteOSTunnelConfig);

  logger.debug() << "TaskDeleteOSTunnelConfig created";
}

TaskDeleteOSTunnelConfig::~TaskDeleteOSTunnelConfig() {
  MZ_COUNT_DTOR(TaskDeleteOSTunnelConfig);
}

void TaskDeleteOSTunnelConfig::run() {
  logger.debug() << "TaskDeleteOSTunnelConfig run";

  Controller* controller = MozillaVPN::instance()->controller();
  Q_ASSERT(controller);

  controller->deleteOSTunnelConfig();
  emit completed();
}
