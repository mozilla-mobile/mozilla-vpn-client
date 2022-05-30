/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "profileflow.h"
#include "tasks/getsubscriptiondetails/taskgetsubscriptiondetails.h"
#include "taskscheduler.h"

namespace {
Logger logger(LOG_MODEL, "ProfileFlow");
}

ProfileFlow::ProfileFlow() { MVPN_COUNT_CTOR(ProfileFlow); }

ProfileFlow::~ProfileFlow() { MVPN_COUNT_DTOR(ProfileFlow); }

void ProfileFlow::setState(State state) {
  logger.debug() << "Set state" << state;

  m_state = state;
  emit stateChanged(m_state);
}

void ProfileFlow::start() {
  logger.debug() << "Start profile flow";

  setState(StateLoading);

  User* user = MozillaVPN::instance()->user();
  Q_ASSERT(user);

  TaskGetSubscriptionDetails* task = new TaskGetSubscriptionDetails(user->email());

  connect(task, &TaskGetSubscriptionDetails::receivedData, [&](const QByteArray& data) {
    subscriptionDetailsFetched(data);
  });

  connect(task, &TaskGetSubscriptionDetails::failed, [&]() {
    logger.debug() << "Task failed";
    // TODO: Remove, only for debugging purposes.
    emit showProfile();
    setState(StateReady);
  });

  TaskScheduler::scheduleTask(task);
}

void ProfileFlow::reset() {
  logger.debug() << "Reset profile flow";

  setState(StateInitial);
}

void ProfileFlow::subscriptionDetailsFetched(
    const QByteArray& subscriptionData) {
  logger.debug() << "Subscription details data fetched";
  Q_UNUSED(subscriptionData);

  if (!m_subscriptionData->fromJson(subscriptionData)) {
    logger.error() << "Failed to parse the Subscription JSON data";
    MozillaVPN::instance()->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  // TODO: Emit signal
  emit showProfile();
  setState(StateReady);
}
