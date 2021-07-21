/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskpasswordauth.h"
#include "hkdf.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "networkmanager.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>

namespace {
Logger logger(LOG_MAIN, "TaskPasswordAuth");
}

TaskPasswordAuth::TaskPasswordAuth(const QString& username,
                                   const QString& password)
    : Task("TaskPasswordAuth"), m_username(username) {
  MVPN_COUNT_CTOR(TaskPasswordAuth);

  // Process the user's password into an FxA auth token
  QString salt =
      QString("identity.mozilla.com/picl/v1/quickStretch:%1").arg(username);
  QByteArray pbkdf = QPasswordDigestor::deriveKeyPbkdf2(
      QCryptographicHash::Sha256, password.toUtf8(), salt.toUtf8(), 1000, 32);

  HKDF hkdf = HKDF(QCryptographicHash::Sha256);
  hkdf.addData(pbkdf);
  m_authpw = hkdf.result(32, "identity.mozilla.com/picl/v1/authPW");
}

TaskPasswordAuth::~TaskPasswordAuth() { MVPN_COUNT_DTOR(TaskPasswordAuth); }

void TaskPasswordAuth::run(MozillaVPN* vpn) {
  Q_UNUSED(vpn);

  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  // Generate a PCKE verification code.
  static QByteArray range(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  for (uint16_t i = 0; i < 128; ++i) {
    m_verifier.append(range.at(generator->generate() % range.length()));
  }

  // Generate the PKCE challenge.
  QByteArray challenge =
      QCryptographicHash::hash(m_verifier, QCryptographicHash::Sha256);

  QUrlQuery query;
  query.addQueryItem("code_challenge", challenge.toBase64());
  query.addQueryItem("code_challenge_method", "S256");
  query.addQueryItem("user_agent", NetworkManager::userAgent());

  QUrl url = NetworkRequest::apiBaseUrl();
  url.setPath("/api/v2/vpn/login/android");
  url.setQuery(query);

  NetworkRequest* request =
      NetworkRequest::createForGetUrl(this, url.toString());

  connect(request, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(request, &NetworkRequest::requestHeaderReceived, this,
          &TaskPasswordAuth::vpnLoginRedirect);
}

void TaskPasswordAuth::authFailed(ErrorHandler::ErrorType error) {
  logger.log() << "Failed to add the device" << error;
  MozillaVPN::instance()->errorHandle(error);
  emit completed();
}

void TaskPasswordAuth::netRequestFailed(QNetworkReply::NetworkError error,
                                        const QByteArray& data) {
  Q_UNUSED(data);
  authFailed(ErrorHandler::toErrorType(error));
}

// In the first stage of password login, we make a request to the VPN service
// to request a login, and they will redirect the client to FxA to handle the
// actual sign-in process.
//
// After making the login request, follow redirects until we get a 200-OK
// status. At which point we will need to gather some parameters from the
// query string being passed to FxA.
//
// Once we have required the login parameters from the redirect, we will need to
// log into FxA to acquire a session token.
void TaskPasswordAuth::vpnLoginRedirect(NetworkRequest* request) {
  // Parse out the FxA authentication details from the redirected URL.
  QUrl url(request->rawHeader("Location"));
  QUrlQuery query(url);

  // Follow redirects until we get a 200 response
  if (request->statusCode() == 302) {
    QUrl url(request->rawHeader("Location"));
    if (!url.isValid()) {
      authFailed(ErrorHandler::ErrorType::RemoteServiceError);
      return;
    }

    // Parse and retain the login query string arguments.
    m_querydata.clear();
    for (auto pair : QUrlQuery(url).queryItems()) {
      m_querydata[pair.first] = pair.second;
    }

    // TODO: We probably want some limit on the maximum number of redirects.
    logger.log() << "Login redirected to" << url.toString();

    NetworkRequest* next =
        NetworkRequest::createForGetUrl(this, url.toString());
    connect(next, &NetworkRequest::requestFailed, this,
            &TaskPasswordAuth::netRequestFailed);
    connect(next, &NetworkRequest::requestHeaderReceived, this,
            &TaskPasswordAuth::vpnLoginRedirect);
    return;
  }

  // On a 200-OK response, move on to the FxA login/authentication API.
  if (request->statusCode() == 200) {
    // TODO check the auth params for correctness...
    NetworkRequest* next = NetworkRequest::createForFxaLogin(
        this, m_username, m_authpw, m_querydata);
    if (!next) {
      authFailed(ErrorHandler::ErrorType::RemoteServiceError);
      return;
    }
    connect(next, &NetworkRequest::requestFailed, this,
            &TaskPasswordAuth::netRequestFailed);
    connect(next, &NetworkRequest::requestCompleted, this,
            &TaskPasswordAuth::fxaLoginComplete);
    return;
  }

  // If anything else occurs, we failed.
  authFailed(ErrorHandler::ErrorType::RemoteServiceError);
}

// Once we have acquired a session token from FxA, we can authorize
// the VPN client using that session token.
void TaskPasswordAuth::fxaLoginComplete(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isNull()) {
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }

  // Extract the FxA login session token
  QJsonObject obj = json.object();
  QJsonValue token = obj.value("sessionToken");
  if (!token.isString()) {
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }
  m_session = QByteArray::fromHex(token.toString().toUtf8());
  logger.log() << "FxA User ID:" << obj.value("uid").toString();
  logger.log() << "FxA Session Token:" << QString(m_session.toHex());

  // Request authorization
  NetworkRequest* next =
      NetworkRequest::createForFxaAuthz(this, m_session, m_querydata);
  connect(next, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(next, &NetworkRequest::requestCompleted, this,
          &TaskPasswordAuth::fxaAuthzComplete);
}

// TODO: Implement Me!
void TaskPasswordAuth::fxaAuthzComplete(const QByteArray& data) {
  Q_UNUSED(data);
  logger.log() << "FxA Authz:" << QString(data);
  authFailed(ErrorHandler::ErrorType::SubscriptionFailureError);
}