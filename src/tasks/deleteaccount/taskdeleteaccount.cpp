/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskdeleteaccount.h"

#include <QJsonObject>

#include "authenticationinapp/authenticationinapp.h"
#include "authenticationinapp/authenticationinappsession.h"
#include "authenticationlistener.h"
#include "errorhandler.h"
#include "logging/logger.h"
#include "networking/networkrequest.h"
#include "taskscheduler.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("TaskDeleteAccount");
}  // anonymous namespace

TaskDeleteAccount::TaskDeleteAccount(const QString& emailAddress)
    : Task("TaskDeleteAccount"), m_emailAddress(emailAddress) {
  MZ_COUNT_CTOR(TaskDeleteAccount);
}

TaskDeleteAccount::~TaskDeleteAccount() { MZ_COUNT_DTOR(TaskDeleteAccount); }

void TaskDeleteAccount::run() {
  logger.debug() << "run";

  Q_ASSERT(!m_authenticationInAppSession);

  QByteArray pkceCodeVerifier;
  QByteArray pkceCodeChallenge;
  AuthenticationListener::generatePkceCodes(pkceCodeVerifier,
                                            pkceCodeChallenge);
  Q_ASSERT(!pkceCodeVerifier.isEmpty() && !pkceCodeChallenge.isEmpty());

  m_authenticationInAppSession = new AuthenticationInAppSession(
      this, AuthenticationInAppSession::TypeAccountDeletion);

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::terminated,
          this, &Task::completed);

  connect(
      m_authenticationInAppSession, &AuthenticationInAppSession::completed,
      this, [this, pkceCodeVerifier](const QString& pkceCodeSuccess) {
        logger.debug() << "Authentication completed with code:"
                       << logger.sensitive(pkceCodeSuccess);

        NetworkRequest* request = new NetworkRequest(this, 200);
        request->post(
            AuthenticationListener::createLoginVerifyUrl(),
            QJsonObject{{"code", pkceCodeSuccess},
                        {"code_verifier", QString(pkceCodeVerifier)}});

        connect(request, &NetworkRequest::requestFailed, this,
                [this](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.error()
                      << "Failed to complete the authentication" << error;
                  REPORTNETWORKERROR(error, ErrorHandler::PropagateError,
                                     name());
                  m_authenticationInAppSession->terminate();
                });

        connect(request, &NetworkRequest::requestCompleted, this,
                [this](const QByteArray&) {
                  logger.debug()
                      << "Authentication completed. Starting the deletion flow";
                  m_authenticationInAppSession->startAccountDeletionFlow();
                });
      });

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::failed,
          this, [this](const ErrorHandler::ErrorType error) {
            REPORTERROR(error, name());
            m_authenticationInAppSession->terminate();
          });

  connect(m_authenticationInAppSession,
          &AuthenticationInAppSession::accountDeleted, this, [this]() {
            m_authenticationInAppSession->terminate();
            emit accountDeleted();
            TaskScheduler::deleteTasks();
          });

  connect(AuthenticationInApp::instance(), &AuthenticationInApp::stateChanged,
          this, [this] {
            switch (AuthenticationInApp::instance()->state()) {
              case AuthenticationInApp::StateSignUp:
                [[fallthrough]];
              case AuthenticationInApp::StateFallbackInBrowser:
                REPORTERROR(ErrorHandler::AuthenticationError, name());
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
