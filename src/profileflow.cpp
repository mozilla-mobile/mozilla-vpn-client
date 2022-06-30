/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "profileflow.h"
#include "models/subscriptiondata.h"
#include "tasks/getsubscriptiondetails/taskgetsubscriptiondetails.h"
#include "taskscheduler.h"
#include "telemetry/gleansample.h"

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

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::profileFlowStateChanged,
      {{"state", QVariant::fromValue(state).toString()}});
}

// Only used for testing and debugging the re-authentication flow
void ProfileFlow::setForceReauthFlow(const bool forceReauthFlow) {
  logger.debug() << "Set force re-authentication:" << forceReauthFlow;
  m_forceReauthFlow = forceReauthFlow;
}

void ProfileFlow::start() {
  logger.debug() << "Start profile flow";

  reset();

  setState(StateLoading);

  User* user = MozillaVPN::instance()->user();
  Q_ASSERT(user);

  TaskGetSubscriptionDetails* task =
      new TaskGetSubscriptionDetails(user->email(), m_forceReauthFlow);

  connect(task, &TaskGetSubscriptionDetails::receivedData, this,
          [this, task](const QByteArray& data) {
            if (task == m_currentTask) {
              m_currentTask = nullptr;
              subscriptionDetailsFetched(data);
            }
          });
  connect(task, &TaskGetSubscriptionDetails::needsAuthentication, this,
          [this, task] {
            if (task == m_currentTask || m_forceReauthFlow) {
              logger.debug() << "Needs authentication";
              setState(StateAuthenticationNeeded);

              // Reset forcing the re-auth flow
              setForceReauthFlow(false);
            }
          });
  connect(task, &TaskGetSubscriptionDetails::failed, this, [this, task]() {
    if (task == m_currentTask) {
      logger.debug() << "Task failed";
      m_currentTask = nullptr;
      setState(StateError);
    }
  });

  TaskScheduler::scheduleTask(task);
  m_currentTask = task;
}

void ProfileFlow::reset() {
  logger.debug() << "Reset profile flow";

  if (m_state != StateInitial) {
    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);
    vpn->cancelReauthentication();

    m_currentTask = nullptr;
    setState(StateInitial);
  }
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
