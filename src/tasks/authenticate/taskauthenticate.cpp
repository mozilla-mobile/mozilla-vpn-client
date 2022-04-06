/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskauthenticate.h"
#include "authenticationlistener.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

namespace {
Logger logger(LOG_MAIN, "TaskAuthenticate");
}  // anonymous namespace

TaskAuthenticate::TaskAuthenticate(
    MozillaVPN::AuthenticationType authenticationType)
    : Task("TaskAuthenticate"), m_authenticationType(authenticationType) {
  MVPN_COUNT_CTOR(TaskAuthenticate);
}

TaskAuthenticate::~TaskAuthenticate() { MVPN_COUNT_DTOR(TaskAuthenticate); }

void TaskAuthenticate::run() {
  logger.debug() << "TaskAuthenticate::Run";

  Q_ASSERT(!m_authenticationListener);

  QByteArray pkceCodeVerifier;
  QByteArray pkceCodeChallenge;
  AuthenticationListener::generatePkceCodes(pkceCodeVerifier,
                                            pkceCodeChallenge);
  Q_ASSERT(!pkceCodeVerifier.isEmpty() && !pkceCodeChallenge.isEmpty());

  m_authenticationListener =
      AuthenticationListener::create(this, m_authenticationType);

  connect(m_authenticationListener, &AuthenticationListener::readyToFinish,
          this, &Task::completed);

  connect(m_authenticationListener, &AuthenticationListener::completed, this,
          [this, pkceCodeVerifier](const QString& pkceCodeSucces) {
            logger.debug() << "Authentication completed with code:"
                           << pkceCodeSucces;

            NetworkRequest* request =
                NetworkRequest::createForAuthenticationVerification(
                    this, pkceCodeSucces, pkceCodeVerifier);

            connect(request, &NetworkRequest::requestFailed, this,
                    [](QNetworkReply::NetworkError error, const QByteArray&) {
                      logger.error()
                          << "Failed to complete the authentication" << error;
                      MozillaVPN::instance()->errorHandle(
                          ErrorHandler::toErrorType(error));
                    });

            connect(request, &NetworkRequest::requestCompleted, this,
                    [this](const QByteArray& data) {
                      logger.debug() << "Authentication completed";
                      authenticationCompleted(data);
                    });
          });

  connect(m_authenticationListener, &AuthenticationListener::failed, this,
          [this](const ErrorHandler::ErrorType error) {
            MozillaVPN::instance()->errorHandle(error);
            m_authenticationListener->aboutToFinish();
          });

  connect(m_authenticationListener, &AuthenticationListener::abortedByUser,
          this, [this]() {
            MozillaVPN::instance()->abortAuthentication();
            m_authenticationListener->aboutToFinish();
          });

  m_authenticationListener->start(this, pkceCodeChallenge,
                                  CODE_CHALLENGE_METHOD);
}

void TaskAuthenticate::authenticationCompleted(const QByteArray& data) {
  logger.debug() << "Authentication completed";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isNull()) {
    vpn->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue userObj = obj.value("user");
  if (!userObj.isObject()) {
    vpn->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  logger.debug() << "User data:"
                 << logger.sensitive(QJsonDocument(userObj.toObject())
                                         .toJson(QJsonDocument::Compact));

  QJsonValue tokenValue = obj.value("token");
  if (!tokenValue.isString()) {
    vpn->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonDocument userDoc;
  userDoc.setObject(userObj.toObject());

  MozillaVPN::instance()->authenticationCompleted(
      userDoc.toJson(QJsonDocument::Compact), tokenValue.toString());

  m_authenticationListener->aboutToFinish();
}
