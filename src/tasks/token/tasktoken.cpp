/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasktoken.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/location.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskToken");
}

TaskToken::TaskToken(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskToken"), m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskToken);
}

TaskToken::~TaskToken() { MZ_COUNT_DTOR(TaskToken); }

void TaskToken::run() {
  // This is a placeholder task to get authentication tokens for MASQUE vpn
  // Now it gets the token form and ENV var, in a realistic scenario it should
  // call Guardian to get a token
  QString token = QString::fromUtf8(qgetenv("MOZVPN_MASQUE_TOKEN"));
  MozillaVPN::instance()->setToken(token);
  emit completed();
  return;
}