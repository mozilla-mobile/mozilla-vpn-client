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
  reset();
  setState(StateLoading);

  User* user = MozillaVPN::instance()->user();
  Q_ASSERT(user);

  TaskGetSubscriptionDetails* task =
      new TaskGetSubscriptionDetails(user->email());
  connect(task, &TaskGetSubscriptionDetails::receivedData, this,
          &ProfileFlow::subscriptionDetailsFetched);
  connect(task, &TaskGetSubscriptionDetails::needsAuthentication, this, [&] {
    logger.debug() << "Needs authentication";
    setState(StateAuthenticating);
    emit showProfile();
  });
  connect(task, &TaskGetSubscriptionDetails::failed, [&]() {
    logger.debug() << "Task failed";

    // TODO: Remove, only for debugging purposes.
    SubscriptionData* subData = SubscriptionData::instance();
    subData->populateFakeData();

    emit showProfile();
    // TODO: Should be `StateInitial`
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

  SubscriptionData* subData = SubscriptionData::instance();
  if (!subData->fromJson(subscriptionData)) {
    logger.error() << "Failed to parse the Subscription JSON data";
    MozillaVPN::instance()->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  // TODO: Remove, only for debugging purposes.
  emit showProfile();
  setState(StateReady);
}