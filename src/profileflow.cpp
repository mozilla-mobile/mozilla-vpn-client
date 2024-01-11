/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "profileflow.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "glean/generated/metrics.h"
#include "logging/logger.h"
#include "models/subscriptiondata.h"
#include "mozillavpn.h"
#include "tasks/getsubscriptiondetails/taskgetsubscriptiondetails.h"
#include "taskscheduler/taskscheduler.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("ProfileFlow");
}

ProfileFlow::ProfileFlow() { MZ_COUNT_CTOR(ProfileFlow); }

ProfileFlow::~ProfileFlow() { MZ_COUNT_DTOR(ProfileFlow); }

void ProfileFlow::setState(State state) {
  logger.debug() << "Set state" << state;

  if (state == StateError) {
    REPORTERROR(ErrorHandler::RemoteServiceError, "profileflow");
  }

  m_state = state;
  emit stateChanged(m_state);

  mozilla::glean::sample::profile_flow_state_changed.record(
      mozilla::glean::sample::ProfileFlowStateChangedExtra{
          ._state = QVariant::fromValue(state).toString(),
      });
}

// Only used for testing and debugging the re-authentication flow
void ProfileFlow::setForceReauthFlow(bool forceReauthFlow) {
  logger.debug() << "Set force re-authentication:" << forceReauthFlow;
  m_forceReauthFlow = forceReauthFlow;
}

void ProfileFlow::start() {
  logger.debug() << "Start profile flow";

  reset();

  setState(StateLoading);

  TaskGetSubscriptionDetails* task = new TaskGetSubscriptionDetails(
      m_forceReauthFlow
          ? TaskGetSubscriptionDetails::ForceAuthenticationFlow
          : TaskGetSubscriptionDetails::RunAuthenticationFlowIfNeeded,
      ErrorHandler::PropagateError);

  connect(task, &TaskGetSubscriptionDetails::needsAuthentication, this,
          [this, task] {
            if (task == m_currentTask || m_forceReauthFlow) {
              logger.debug() << "Needs authentication";
              setState(StateAuthenticationNeeded);

              // Reset forcing the re-auth flow
              setForceReauthFlow(false);
            }
          });

  connect(task, &TaskGetSubscriptionDetails::operationCompleted, this,
          [this, task](bool status) {
            if (task != m_currentTask) {
              return;
            }

            m_currentTask = nullptr;
            setState(status ? StateReady : StateError);
          });

  connect(MozillaVPN::instance(), &MozillaVPN::stateChanged, this, [this]() {
    if (App::instance()->state() != App::StateMain) {
      reset();
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
