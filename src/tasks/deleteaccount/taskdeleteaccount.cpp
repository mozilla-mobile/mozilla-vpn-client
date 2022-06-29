/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskdeleteaccount.h"
#include "authenticationlistener.h"
#include "authenticationinapp/authenticationinappsession.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "taskscheduler.h"

namespace {
Logger logger(LOG_MAIN, "TaskDeleteAccount");
}  // anonymous namespace

TaskDeleteAccount::TaskDeleteAccount(const QString& emailAddress)
    : Task("TaskDeleteAccount"), m_emailAddress(emailAddress) {
  MVPN_COUNT_CTOR(TaskDeleteAccount);
}

TaskDeleteAccount::~TaskDeleteAccount() { MVPN_COUNT_DTOR(TaskDeleteAccount); }

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
      this, [this, pkceCodeVerifier](const QString& pkceCodeSucces) {
        logger.debug() << "Authentication completed with code:"
                       << logger.sensitive(pkceCodeSucces);

        NetworkRequest* request =
            NetworkRequest::createForAuthenticationVerification(
                this, pkceCodeSucces, pkceCodeVerifier);

        connect(request, &NetworkRequest::requestFailed, this,
                [this](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.error()
                      << "Failed to complete the authentication" << error;
                  MozillaVPN::instance()->errorHandle(
                      ErrorHandler::toErrorType(error));
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
            MozillaVPN::instance()->errorHandle(error);
            m_authenticationInAppSession->terminate();
          });

  connect(m_authenticationInAppSession,
          &AuthenticationInAppSession::accountDeleted, this, [this]() {
            m_authenticationInAppSession->terminate();
            TaskScheduler::deleteTasks();
            emit MozillaVPN::instance()->accountDeleted();
          });

  m_authenticationInAppSession->start(this, pkceCodeChallenge,
                                      CODE_CHALLENGE_METHOD, m_emailAddress);
}
