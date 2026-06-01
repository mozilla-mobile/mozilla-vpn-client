/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetsubscriptiondetails.h"

#include <QJsonObject>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/subscriptiondata.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskGetSubscriptionDetails");
}  // anonymous namespace

TaskGetSubscriptionDetails::TaskGetSubscriptionDetails(
    AuthenticationPolicy authenticationPolicy,
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskGetSubscriptionDetails"),
      m_authenticationPolicy(authenticationPolicy),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskGetSubscriptionDetails);
}

TaskGetSubscriptionDetails::~TaskGetSubscriptionDetails() {
  MZ_COUNT_DTOR(TaskGetSubscriptionDetails);
}

void TaskGetSubscriptionDetails::run() {
  logger.debug() << "run";

  NetworkRequest* request = new NetworkRequest(this, 200);
  request->auth();
  request->get(Constants::apiUrl(Constants::SubscriptionDetails));

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Get subscription details failed" << error;

        if (error != QNetworkReply::AuthenticationRequiredError) {
          REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
        } else {
          switch (m_authenticationPolicy) {
            case RunAuthenticationFlowIfNeeded:
              logger.info() << "Starting web-based re-authentication.";
              emit mustTransitionAuthToWeb();
              emit completed();
              return;

              logger.error() << "Network request failed after authentication";
              break;

            case NoAuthenticationFlow:
              logger.error() << "Needs authentication, but excluded by policy";
              break;
          }
        }

        maybeComplete(false);
      });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get subscription details completed"
                           << logger.sensitive(data);

            MozillaVPN* vpn = MozillaVPN::instance();
            Q_ASSERT(vpn);

            if (!vpn->subscriptionData()->fromJson(data)) {
              logger.error() << "Failed to parse the Subscription JSON data";
              maybeComplete(false);
              return;
            }

            vpn->subscriptionData()->writeSettings();
            maybeComplete(true);
          });
}

void TaskGetSubscriptionDetails::maybeComplete(bool status) {
  if (!status) {
    MozillaVPN::instance()->subscriptionData()->resetData();
  }

  emit operationCompleted(status);
  emit completed();
}
