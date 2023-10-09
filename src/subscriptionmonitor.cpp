/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptionmonitor.h"

#include <QCoreApplication>

#include "connectionmanager.h"
#include "constants.h"
#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/account/taskaccount.h"
#include "taskscheduler.h"

namespace {
Logger logger("SubscriptionMonitor");
}

SubscriptionMonitor::SubscriptionMonitor(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(SubscriptionMonitor);

  /*
   * Here we setup 2 connections:
   * ConnectionHealth: to monitor No Signal state
   * Controller: to monitor when user turns their VPN off
   * so we can ping guardian and then show the subscription expired page.
   */

  connect(MozillaVPN::instance()->connectionHealth(),
          &ConnectionHealth::stabilityChanged, this, [this]() {
            logger.debug() << "VPN connection stability has changed";
            if (MozillaVPN::instance()->connectionManager()->isVPNActive()) {
              m_lastKnownStabilityState =
                  MozillaVPN::instance()->connectionHealth()->stability();
            }
          });

  connect(MozillaVPN::instance()->connectionManager(),
          &ConnectionManager::stateChanged, this, [this]() {
            if (!MozillaVPN::instance()->connectionManager()->isVPNActive() &&
                m_lastKnownStabilityState ==
                    ConnectionHealth::ConnectionStability::NoSignal) {
              logger.debug() << "User has toggled the VPN off after No Signal";
              TaskScheduler::scheduleTask(
                  new TaskAccount(ErrorHandler::DoNotPropagateError));

              // Reset the state tracker
              m_lastKnownStabilityState =
                  ConnectionHealth::ConnectionStability::Stable;
            }
          });
}

SubscriptionMonitor::~SubscriptionMonitor() {
  MZ_COUNT_DTOR(SubscriptionMonitor);
}

// static
SubscriptionMonitor* SubscriptionMonitor::instance() {
  static SubscriptionMonitor* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new SubscriptionMonitor(qApp);
  }
  return s_instance;
}
