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

#include <QCryptographicHash>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>

constexpr const char* CODE_CHALLENGE_METHOD = "S256";

namespace {

Logger logger(LOG_MAIN, "TaskAuthenticate");

QByteArray generatePkceCodeVerifier() {
  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  QByteArray pkceCodeVerifier;
  static QByteArray range(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  for (uint16_t i = 0; i < 128; ++i) {
    pkceCodeVerifier.append(range.at(generator->generate() % range.length()));
  }

  return pkceCodeVerifier;
}

}  // anonymous namespace

TaskAuthenticate::TaskAuthenticate(
    MozillaVPN::AuthenticationType authenticationType)
    : Task("TaskAuthenticate"), m_authenticationType(authenticationType) {
  MVPN_COUNT_CTOR(TaskAuthenticate);
  Q_ASSERT(authenticationType != MozillaVPN::DefaultAuthentication);
}

TaskAuthenticate::~TaskAuthenticate() { MVPN_COUNT_DTOR(TaskAuthenticate); }

void TaskAuthenticate::run(MozillaVPN* vpn) {
  logger.debug() << "TaskAuthenticate::Run";

  Q_ASSERT(!m_authenticationListener);

  QByteArray pkceCodeVerifier = generatePkceCodeVerifier();
  QByteArray pkceCodeChallenge =
      QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Sha256)
          .toBase64();
  Q_ASSERT(pkceCodeChallenge.length() == 44);

  m_authenticationListener =
      AuthenticationListener::create(this, m_authenticationType);

  connect(
      m_authenticationListener, &AuthenticationListener::completed,
      [this, vpn, pkceCodeVerifier](const QString& pkceCodeSucces) {
        logger.debug() << "Authentication completed with code:" << pkceCodeSucces;

        NetworkRequest* request =
            NetworkRequest::createForAuthenticationVerification(
                this, pkceCodeSucces, pkceCodeVerifier);

        connect(request, &NetworkRequest::requestFailed,
                [vpn](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.error()
                      << "Failed to complete the authentication" << error;
                  vpn->errorHandle(ErrorHandler::toErrorType(error));
                });

        connect(request, &NetworkRequest::requestCompleted,
                [this, vpn](const QByteArray& data) {
                  logger.debug() << "Authentication completed";
                  authenticationCompleted(vpn, data);
                });
      });

  connect(m_authenticationListener, &AuthenticationListener::failed,
          [this, vpn](const ErrorHandler::ErrorType error) {
            vpn->errorHandle(error);
            emit completed();
          });

  connect(m_authenticationListener, &AuthenticationListener::abortedByUser,
          [this, vpn]() {
            vpn->abortAuthentication();
            emit completed();
          });

  m_authenticationListener->start(pkceCodeChallenge, CODE_CHALLENGE_METHOD);
}

void TaskAuthenticate::authenticationCompleted(MozillaVPN* vpn,
                                               const QByteArray& data) {
  logger.debug() << "Authentication completed";

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

#ifdef QT_DEBUG
  logger.debug()
      << "User data:"
      << QJsonDocument(userObj.toObject()).toJson(QJsonDocument::Compact);
#endif

  QJsonValue tokenValue = obj.value("token");
  if (!tokenValue.isString()) {
    vpn->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonDocument userDoc;
  userDoc.setObject(userObj.toObject());

  vpn->authenticationCompleted(userDoc.toJson(QJsonDocument::Compact),
                               tokenValue.toString());

  emit completed();
}
