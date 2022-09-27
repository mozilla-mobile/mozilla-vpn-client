/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetsubscriptiondetails.h"
#include "authenticationlistener.h"
#include "authenticationinapp/authenticationinapp.h"
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
    const QString& emailAddress, const bool forceReauth)
    : Task("TaskGetSubscriptionDetails"),
      m_emailAddress(emailAddress),
      m_forceReauth(forceReauth) {
  MVPN_COUNT_CTOR(TaskGetSubscriptionDetails);
}

TaskGetSubscriptionDetails::~TaskGetSubscriptionDetails() {
  MVPN_COUNT_DTOR(TaskGetSubscriptionDetails);
}

void TaskGetSubscriptionDetails::run() {
  logger.debug() << "run";

  // If this Task is created with `forceReauth = true`,
  // we are forcing the user to authenticate.
  if (m_forceReauth) {
    logger.error() << "Force authentication";
    initAuthentication();
    // Reset the flag so we can run the Task after successful authentication.
    m_forceReauth = false;
    return;
  }

  NetworkRequest* request =
      NetworkRequest::createForGetSubscriptionDetails(this);

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Get subscription details failed" << error;

        // Network request failed after authentication for a second time
        if (m_authenticationInAppSession) {
          logger.error() << "Network request failed after authentication";

          MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
          emit failed();
          m_authenticationInAppSession->terminate();
          return;
        }

        // User needs to (re)authenticate
        if (error == QNetworkReply::AuthenticationRequiredError) {
          logger.error() << "Needs authentication";
          initAuthentication();
          return;
        }

        MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));

        // We need to emit two separate signals here.
        // `failed`: Signal for connected objects that are monitoring this Task
        // `completed`: Notify `TaskScheduler` that this Task can be deleted
        emit failed();
        emit completed();
      });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get subscription details completed"
                           << logger.sensitive(data);

            emit receivedData(data);

            if (m_authenticationInAppSession) {
              m_authenticationInAppSession->terminate();
              return;
            }
            emit completed();
          });
}

void TaskGetSubscriptionDetails::initAuthentication() {
  logger.debug() << "Init authentication";
  Q_ASSERT(!m_authenticationInAppSession);

  emit needsAuthentication();

  QByteArray pkceCodeVerifier;
  QByteArray pkceCodeChallenge;
  AuthenticationListener::generatePkceCodes(pkceCodeVerifier,
                                            pkceCodeChallenge);
  Q_ASSERT(!pkceCodeVerifier.isEmpty() && !pkceCodeChallenge.isEmpty());

  m_authenticationInAppSession = new AuthenticationInAppSession(
      this, AuthenticationInAppSession::TypeSubscriptionManagement);

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::terminated,
          this, &Task::completed);

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::completed,
          this, [this, pkceCodeVerifier](const QString& pkceCodeSuccess) {
            logger.debug() << "Authentication completed with code:"
                           << logger.sensitive(pkceCodeSuccess);

            NetworkRequest* request =
                NetworkRequest::createForAuthenticationVerification(
                    this, pkceCodeSuccess, pkceCodeVerifier);

            connect(
                request, &NetworkRequest::requestFailed, this,
                [this](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.error()
                      << "Failed to complete the authentication" << error;
                  m_authenticationInAppSession->terminate();
                });

            connect(request, &NetworkRequest::requestCompleted, this,
                    [this](const QByteArray&) {
                      logger.debug() << "Authentication completed. Attempt "
                                        "fetching subscription details again.";
                      run();
                    });
          });

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::failed,
          this, [this](const ErrorHandler::ErrorType error) {
            MozillaVPN::instance()->errorHandle(error);
            m_authenticationInAppSession->terminate();
          });

  connect(AuthenticationInApp::instance(), &AuthenticationInApp::stateChanged,
          this, [this] {
            switch (AuthenticationInApp::instance()->state()) {
              case AuthenticationInApp::StateSignUp:
                [[fallthrough]];
              case AuthenticationInApp::StateFallbackInBrowser:
                MozillaVPN::instance()->errorHandle(
                    ErrorHandler::AuthenticationError);
                emit failed();
                m_authenticationInAppSession->terminate();
                break;

              default:
                // All the other states should be handled by the front-end code.
                break;
            }
          });

  m_authenticationInAppSession->start(this, pkceCodeChallenge,
                                      CODE_CHALLENGE_METHOD, m_emailAddress);
}
