/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "taskscheduler.h"
#include "purchasewebhandler.h"

#include <QCoreApplication>

namespace {
Logger logger(LOG_IAP, "PurchaseWebHandler");
}  // namespace

PurchaseWebHandler::PurchaseWebHandler(QObject* parent)
    : PurchaseHandler(parent) {
  MVPN_COUNT_CTOR(PurchaseWebHandler);
}

PurchaseWebHandler::~PurchaseWebHandler() {
  MVPN_COUNT_DTOR(PurchaseWebHandler);
}

void PurchaseWebHandler::subscribe(const QString& productIdentifier) {
  logger.debug() << "Subscription initiated.";
  emit subscriptionStarted(productIdentifier);
}

void PurchaseWebHandler::startSubscription() {
  m_subscriptionState = eActive;
  logger.debug() << "Starting subscription on web.";
  // Although we are already logged in, the mechanism on guardian for getting to
  // a web subscription is via the login endpoint. Additionally, we need the
  // user to login on the browser (rather than the client) in order to complete
  // the subscription platform flow elegantly. If/when guardian adds endpoints
  // that seperate these concerns we can use them.
  TaskScheduler::scheduleTask(new TaskAuthenticate(
      MozillaVPN::AuthenticationType::AuthenticationInBrowser));
}

void PurchaseWebHandler::stopSubscription() {
  logger.debug() << "Stop subscription";
  m_subscriptionState = eInactive;
}
