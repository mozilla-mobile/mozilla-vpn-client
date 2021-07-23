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

  HKDF hash(QCryptographicHash::Sha256);
  hash.addData(pbkdf);
  m_authpw = hash.result(32, "identity.mozilla.com/picl/v1/authPW");
}

TaskPasswordAuth::~TaskPasswordAuth() { MVPN_COUNT_DTOR(TaskPasswordAuth); }

void TaskPasswordAuth::run(MozillaVPN* vpn) {
  Q_UNUSED(vpn);

  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  // Generate a PCKE verification code.
  static QByteArray range(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  m_verifier.clear();
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

  // Login using the android auth flow to convince the API to supply the OAuth
  // code as a query parameter during redirection instead of invoking a browser
  QUrl url = NetworkRequest::apiBaseUrl();
  url.setPath("/api/v2/vpn/login/android");
  url.setQuery(query);

  NetworkRequest* request =
      NetworkRequest::createForGetUrl(this, url.toString(), 200);

  connect(request, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(request, &NetworkRequest::requestHeaderReceived, this,
          &TaskPasswordAuth::vpnLoginHeaders);
}

void TaskPasswordAuth::authFailed(ErrorHandler::ErrorType error) {
  logger.log() << "Authentication failed" << error;
  MozillaVPN::instance()->errorHandle(ErrorHandler::AuthenticationError);
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
// After making the login request, we will gather the authentication parameters
// from query string provided in the redirection. We can use them to perform
// authentication manually using the FxA API instead of needing a browser.
void TaskPasswordAuth::vpnLoginHeaders(NetworkRequest* request) {
  // Parse the authentication parameters from the redirected query string.
  m_querydata.clear();
  for (auto pair : QUrlQuery(request->url()).queryItems()) {
    m_querydata[pair.first] = pair.second;
  }

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
}

// After logging into FxA, we should receive a session token, which can
// then be used to authorize the VPN client.
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
#ifdef QT_DEBUG
  logger.log() << "FxA Session Token:" << QString(m_session.toHex());
#endif

  // Request authorization
  NetworkRequest* next =
      NetworkRequest::createForFxaAuthz(this, m_session, m_querydata);
  connect(next, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(next, &NetworkRequest::requestCompleted, this,
          &TaskPasswordAuth::fxaAuthzComplete);
}

// After authorization has completed, FxA will provide us with an authorization
// code, and a redirection target. We need to follow the redirection target in
// order to pass our authorization onto the VPN service.
void TaskPasswordAuth::fxaAuthzComplete(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isNull()) {
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue code = obj.value("code");
  if (!code.isString()) {
    logger.log() << "FxA Authz: code not found";
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }
  QJsonValue state = obj.value("state");
  if (!state.isString()) {
    logger.log() << "FxA Authz: state not found";
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }
  QJsonValue redirect = obj.value("redirect");
  if (!redirect.isString()) {
    logger.log() << "FxA Authz: redirect not found";
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }

  NetworkRequest* next =
      NetworkRequest::createForGetUrl(this, redirect.toString(), 200);

  connect(next, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(next, &NetworkRequest::requestHeaderReceived, this,
          &TaskPasswordAuth::fxaAuthzHeaders);
}

// Because we used the android auth flow, the VPN services will provide us
// with the OAuth code in the query string after yet-another redirection. To
// turn it into an OAuth token, we simply need to POST to the verification
// endpoint.
void TaskPasswordAuth::fxaAuthzHeaders(NetworkRequest* request) {
  // On a 200 response, we receive the OAuth code from the query string
  QString code = QUrlQuery(request->url()).queryItemValue("code");
  if (code.isEmpty()) {
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }

  NetworkRequest* next = NetworkRequest::createForAuthenticationVerification(
      this, code, m_verifier);
  connect(next, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(next, &NetworkRequest::requestCompleted, this,
          &TaskPasswordAuth::vpnVerifyComplete);
}

void TaskPasswordAuth::vpnVerifyComplete(const QByteArray& data) {
  logger.log() << "Authentication completed";
  MozillaVPN* vpn = MozillaVPN::instance();

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isNull()) {
    authFailed(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue userObj = obj.value("user");
  if (!userObj.isObject()) {
    authFailed(ErrorHandler::RemoteServiceError);
    return;
  }

#ifdef QT_DEBUG
  logger.log()
      << "User data:"
      << QJsonDocument(userObj.toObject()).toJson(QJsonDocument::Compact);
#endif

  QJsonValue tokenValue = obj.value("token");
  if (!tokenValue.isString()) {
    authFailed(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonDocument userDoc;
  userDoc.setObject(userObj.toObject());

  vpn->authenticationCompleted(userDoc.toJson(QJsonDocument::Compact),
                               tokenValue.toString());

  emit completed();
}
