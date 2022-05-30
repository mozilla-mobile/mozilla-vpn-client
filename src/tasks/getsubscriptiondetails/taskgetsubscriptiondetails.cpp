/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetsubscriptiondetails.h"
#include "authenticationlistener.h"
#include "authenticationinapp/authenticationinappsession.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskGetSubscriptionDetails");
}  // anonymous namespace

TaskGetSubscriptionDetails::TaskGetSubscriptionDetails(
    const QString& emailAddress)
    : Task("TaskGetSubscriptionDetails"), m_emailAddress(emailAddress) {
  MVPN_COUNT_CTOR(TaskGetSubscriptionDetails);
}

TaskGetSubscriptionDetails::~TaskGetSubscriptionDetails() {
  MVPN_COUNT_DTOR(TaskGetSubscriptionDetails);
}

void TaskGetSubscriptionDetails::run() {
  logger.debug() << "run";

  NetworkRequest* request =
      NetworkRequest::createForGetSubscriptionDetails(this);

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Get subscription details failed" << error;

        emit failed();
        emit completed();

        // // Network request failed after authentication for a second time
        // if (m_authenticationInAppSession) {
        //   MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        //   emit failed();
        //   m_authenticationInAppSession->terminate();
        //   return;
        // }

        // // User needs to (re)authenticate
        // if (error == QNetworkReply::AuthenticationRequiredError) {
        //   needsAuthentication();
        //   return;
        // }

        // MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        // emit failed();
        // emit completed();
      });

  // connect(request, &NetworkRequest::requestCompleted, this,
  //         [this](const QByteArray& data) {
  //           logger.debug() << "Get subscription details completed" << data;

  //           emit receivedData(data);

  //           if (m_authenticationInAppSession) {
  //             m_authenticationInAppSession->terminate();
  //             return;
  //           }
  //           emit completed();
  //         });
}

void TaskGetSubscriptionDetails::needsAuthentication() {
  Q_ASSERT(!m_authenticationInAppSession);

  // QByteArray pkceCodeVerifier;
  // QByteArray pkceCodeChallenge;
  // AuthenticationListener::generatePkceCodes(pkceCodeVerifier,
  //                                           pkceCodeChallenge);
  // Q_ASSERT(!pkceCodeVerifier.isEmpty() && !pkceCodeChallenge.isEmpty());

  // m_authenticationInAppSession = new AuthenticationInAppSession(this);

  // connect(m_authenticationInAppSession, &AuthenticationInAppSession::terminated,
  //         this, &Task::completed);

  // connect(m_authenticationInAppSession, &AuthenticationInAppSession::completed,
  //         this, [this, pkceCodeVerifier](const QString& pkceCodeSuccess) {
  //           logger.debug() << "Authentication completed with code:"
  //                          << logger.sensitive(pkceCodeSuccess);

  //           NetworkRequest* request =
  //               NetworkRequest::createForAuthenticationVerification(
  //                   this, pkceCodeSuccess, pkceCodeVerifier);

  //           connect(
  //               request, &NetworkRequest::requestFailed, this,
  //               [this](QNetworkReply::NetworkError error, const QByteArray&) {
  //                 logger.error()
  //                     << "Failed to complete the authentication" << error;
  //                 m_authenticationInAppSession->terminate();
  //               });

  //           connect(request, &NetworkRequest::requestCompleted, this,
  //                   [this](const QByteArray&) {
  //                     logger.debug() << "Authentication completed. Attempt "
  //                                       "fetching subscription details again.";
  //                     run();
  //                   });
  //         });

  // connect(m_authenticationInAppSession, &AuthenticationInAppSession::failed,
  //         this, [this](const ErrorHandler::ErrorType error) {
  //           MozillaVPN::instance()->errorHandle(error);
  //           m_authenticationInAppSession->terminate();
  //         });

  // m_authenticationInAppSession->start(this, pkceCodeChallenge,
  //                                     CODE_CHALLENGE_METHOD, m_emailAddress);
}
