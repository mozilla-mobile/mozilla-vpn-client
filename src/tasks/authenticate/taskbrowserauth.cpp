/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskbrowserauth.h"
#include "authenticationlistener.h"
#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "networkmanager.h"

#include <QCryptographicHash>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>

namespace {

Logger logger(LOG_MAIN, "TaskBrowserAuth");

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

TaskBrowserAuth::TaskBrowserAuth() : Task("TaskBrowserAuth") {
  MVPN_COUNT_CTOR(TaskBrowserAuth);
}

TaskBrowserAuth::~TaskBrowserAuth() { MVPN_COUNT_DTOR(TaskBrowserAuth); }

void TaskBrowserAuth::run(MozillaVPN* vpn) {
  Q_ASSERT(vpn);
  logger.log() << "TaskBrowserAuth::Run";

  Q_ASSERT(!m_authenticationListener);

  QByteArray pkceCodeVerifier = generatePkceCodeVerifier();
  QByteArray pkceCodeChallenge =
      QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Sha256)
          .toBase64();
  Q_ASSERT(pkceCodeChallenge.length() == 44);

  m_authenticationListener = AuthenticationListener::create(this);

  connect(
      m_authenticationListener, &AuthenticationListener::completed,
      [this, vpn, pkceCodeVerifier](const QString& pkceCodeSucces) {
        logger.log() << "Authentication completed with code:" << pkceCodeSucces;

        NetworkRequest* request =
            NetworkRequest::createForAuthenticationVerification(
                this, pkceCodeSucces, pkceCodeVerifier);

        connect(request, &NetworkRequest::requestFailed,
                [vpn](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.log()
                      << "Failed to complete the authentication" << error;
                  vpn->errorHandle(ErrorHandler::toErrorType(error));
                });

        connect(request, &NetworkRequest::requestCompleted,
                [this, vpn](const QByteArray& data) {
                  logger.log() << "Authentication completed";
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

  QString path("/api/v2/vpn/login/");

#if !defined(MVPN_DUMMY)
  path.append(Constants::PLATFORM_NAME);
#else
  // Let's use linux here.
  path.append("linux");
#endif

  QUrl url(NetworkRequest::apiBaseUrl());
  url.setPath(path);

  QUrlQuery query;
  query.addQueryItem("code_challenge",
                     QUrl::toPercentEncoding(pkceCodeChallenge));
  query.addQueryItem("code_challenge_method", "S256");
  query.addQueryItem("user_agent", NetworkManager::userAgent());

  m_authenticationListener->start(vpn, url, query);
}

void TaskBrowserAuth::authenticationCompleted(MozillaVPN* vpn,
                                              const QByteArray& data) {
  logger.log() << "Authentication completed";

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
  logger.log()
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
