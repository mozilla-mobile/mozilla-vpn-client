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

  if (state == StateError) {
    MozillaVPN::instance()->errorHandle(ErrorHandler::RemoteServiceError);
  }

  m_state = state;
  emit stateChanged(m_state);
}

void ProfileFlow::start() {
  logger.debug() << "Start profile flow";

  if (m_state != StateInitial) {
    return;
  }

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
    setState(StateError);
  });

  TaskScheduler::scheduleTask(task);
}

void ProfileFlow::reset() {
  logger.debug() << "Reset profile flow";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);
  vpn->cancelReauthentication();

  setState(StateInitial);
}

void ProfileFlow::subscriptionDetailsFetched(
    const QByteArray& subscriptionData) {
  logger.debug() << "Subscription details data fetched";

  if (!MozillaVPN::instance()->subscriptionData()->fromJson(subscriptionData)) {
    logger.error() << "Failed to parse the Subscription JSON data";
    setState(StateError);
    return;
  }

  setState(StateReady);
}

// Only used for testing
void ProfileFlow::showFakeData() {
  logger.debug() << "Show fake data";
  Q_ASSERT(!Constants::inProduction());

  MozillaVPN::instance()->subscriptionData()->populateFakeData();
  setState(StateReady);
}
