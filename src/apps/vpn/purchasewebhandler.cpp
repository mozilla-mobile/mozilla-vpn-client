/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "purchasewebhandler.h"

#include "leakdetector.h"
#include "logger.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "taskscheduler.h"

namespace {
Logger logger("PurchaseWebHandler");
}  // namespace

PurchaseWebHandler::PurchaseWebHandler(QObject* parent)
    : PurchaseHandler(parent) {
  MZ_COUNT_CTOR(PurchaseWebHandler);
}

PurchaseWebHandler::~PurchaseWebHandler() { MZ_COUNT_DTOR(PurchaseWebHandler); }

void PurchaseWebHandler::startSubscription(const QString&) {
  if (m_subscriptionState != eInactive) {
    logger.warning() << "We're already subscribing.";
    return;
  }
  m_subscriptionState = eActive;
  logger.debug() << "Starting the subscription";

  // Although we are already logged in, the mechanism on guardian for getting to
  // a web subscription is via the login endpoint. Additionally, we need the
  // user to login on the browser (rather than the client) in order to complete
  // the subscription platform flow elegantly. If/when guardian adds endpoints
  // that seperate these concerns we can use them.
  TaskScheduler::scheduleTask(new TaskAuthenticate(
      MozillaVPN::AuthenticationType::AuthenticationInBrowser));
}

void PurchaseWebHandler::cancelSubscription() {
  logger.debug() << "Cancel subscription";
  m_subscriptionState = eInactive;
  // We delete all tasks so that we clean-up the authentication task we started
  // above.
  // TODO - Do we have a mechanism to just clean-up the one task?
  TaskScheduler::deleteTasks();
  emit subscriptionCanceled();
}

void PurchaseWebHandler::startRestoreSubscription() {
  logger.error() << "Restore not implemented!";
  emit subscriptionFailed();
}

void PurchaseWebHandler::nativeRegisterProducts() {
  logger.error()
      << "nativeRegisterProducts should not be called for PurchaseWebHandler";
  Q_ASSERT(false);
}
