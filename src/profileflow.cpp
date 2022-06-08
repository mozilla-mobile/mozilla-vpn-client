/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "profileflow.h"
#include "subscriptiondata.h"
#include "tasks/getsubscriptiondetails/taskgetsubscriptiondetails.h"
#include "taskscheduler.h"

#include <QJsonDocument>
#include <QJsonObject>

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

  TaskGetSubscriptionDetails* task =
      new TaskGetSubscriptionDetails(user->email());
  connect(task, &TaskGetSubscriptionDetails::receivedData, this,
          &ProfileFlow::subscriptionDetailsFetched);
  connect(task, &TaskGetSubscriptionDetails::needsAuthentication, this, [&] {
    logger.debug() << "Needs authentication";
    setState(StateAuthenticationNeeded);
  });
  connect(task, &TaskGetSubscriptionDetails::failed, [&]() {
    logger.debug() << "Task failed";

    // TODO: Remove, only for debugging purposes.
    MozillaVPN::instance()->subscriptionData()->populateFakeData();

    // TODO: Should be `StateInitial`
    setState(StateReady);
  });

  TaskScheduler::scheduleTask(task);
}

void ProfileFlow::subscriptionDetailsFetched(
    const QByteArray& subscriptionData) {
  logger.debug() << "Subscription details data fetched";

  if (!MozillaVPN::instance()->subscriptionData()->fromJson(subscriptionData)) {
    logger.error() << "Failed to parse the Subscription JSON data";
    MozillaVPN::instance()->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  setState(StateReady);
}