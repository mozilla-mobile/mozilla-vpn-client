/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptionmonitor.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <QMetaObject>

#include "appconstants.h"
#include "connectionhealth.h"
#include "controller.h"
#include "dnshelper.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "taskscheduler.h"

namespace {
Logger logger("SubscriptionMonitor");
}

SubscriptionMonitor::SubscriptionMonitor(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(SubscriptionMonitor);

/**
 * Here we need to setup 2 connections:
 * connection health: to monitor no signal state
 * controller: to monitor if/when user turns their VPN off 
 *             so we can ping guardian and then show the sub expired page.
 */

  connect(MozillaVPN::instance()->connectionHealth(), &ConnectionHealth::stabilityChanged, this, [] () {
    if (MozillaVPN::instance()->connectionHealth()->stability() == ConnectionHealth::NoSignal)
    {
      logger.debug() << "VPN connection stability is NoSignal";
      //What else do I need to do here?
    }
  });
 
  connect(MozillaVPN::instance()->controller(), &Controller::stateChanged, this,
          [this]() {
            Controller::State state =
                MozillaVPN::instance()->controller()->state();
            // m_operationRunning is set to true when the Controller is in
            // StateOn. So, if we see a change, it means that the new settings
            // values have been taken in consideration. We are ready to
            // schedule a new TaskControllerAction if needed.
            if (state == Controller::StateOff) {
              logger.debug() << "User has toggled the VPN off";
              m_operationRunning = false; //not sure we need this

              // Check the subscription status. setStatus() ia private 
              // so I think instead of setting controller status, we can just go ahead and send in the network request directly
              // MozillaVPN::instance()->controller()->setState(Controller::StateCheckSubscription); // Can't do this here
              TaskFunction* task = new TaskFunction([]() {});
              NetworkRequest* request = new NetworkRequest(task, 200);
              request->auth(MozillaVPN::authorizationHeader());
              request->get(AppConstants::apiUrl(AppConstants::Account));

              // TODO: We most likely need to listen for request failures
              // connect(request, &NetworkRequest::requestFailed, this,
              //         [this](QNetworkReply::NetworkError error, const QByteArray&) {
              //           logger.error() << "Account request failed" << error;
              //           REPORTNETWORKERROR(error, ErrorHandler::DoNotPropagateError,
              //                             "PreActivationSubscriptionCheck");
            }
          });
}

SubscriptionMonitor::~SubscriptionMonitor() { MZ_COUNT_DTOR(SubscriptionMonitor); }

// static
SubscriptionMonitor* SubscriptionMonitor::instance() {
  static SubscriptionMonitor* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new SubscriptionMonitor(qApp);
  }
  return s_instance;
}

void SubscriptionMonitor::operationCompleted() { m_operationRunning = false; }
