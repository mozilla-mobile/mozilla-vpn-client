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

constexpr int AUTH_REDIRECT_LIMIT = 5;

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

  QUrl url = NetworkRequest::apiBaseUrl();
  url.setPath("/api/v2/vpn/login/android");
  url.setQuery(query);

  m_redirectlimit = AUTH_REDIRECT_LIMIT;
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
  // Follow redirects until we get a 200 response
  if (request->statusCode() == 302) {
    followRedirect(request, &TaskPasswordAuth::vpnLoginRedirect);
    return;
  }

  // On a 200-OK response, move on to the FxA login/authentication API.
  if (request->statusCode() == 200) {
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

  m_redirectlimit = AUTH_REDIRECT_LIMIT;
  NetworkRequest* next =
      NetworkRequest::createForGetUrl(this, redirect.toString());

  connect(next, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(next, &NetworkRequest::requestHeaderReceived, this,
          &TaskPasswordAuth::fxaAuthzRedirect);
}

void TaskPasswordAuth::fxaAuthzRedirect(NetworkRequest* request) {
  logger.log() << "FxA Authz: redirect returned" << request->statusCode();

  // Follow redirects until we get a 200 response
  if ((request->statusCode() >= 300) && (request->statusCode() < 400)) {
    logger.log() << "Authorization redirected to" << request->url().toString();
    followRedirect(request, &TaskPasswordAuth::fxaAuthzRedirect);
    return;
  }

  // On a 200 response, we receive the OAuth code from the query string
  if (request->statusCode() == 200) {
    QString code = QUrlQuery(request->url()).queryItemValue("code");
    if (code.isEmpty()) {
      authFailed(ErrorHandler::ErrorType::RemoteServiceError);
      return;
    }

    // TODO: And finally, we need to submit the OAuth code to receive the token.
    NetworkRequest* next = NetworkRequest::createForAuthenticationVerification(
        this, code, m_verifier);
    connect(next, &NetworkRequest::requestFailed, this,
            &TaskPasswordAuth::netRequestFailed);
    connect(next, &NetworkRequest::requestCompleted, this,
            &TaskPasswordAuth::vpnVerifyComplete);
    return;
  }

  // If anything else occurs, we failed.
  authFailed(ErrorHandler::RemoteServiceError);
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

void TaskPasswordAuth::followRedirect(
    const NetworkRequest* request,
    void (TaskPasswordAuth::*callback)(NetworkRequest*)) {
  Q_ASSERT(request->statusCode() >= 300);
  Q_ASSERT(request->statusCode() < 400);

  QString location = QString(request->rawHeader("Location"));
  QUrl target = request->url().resolved(location);
  if (location.isEmpty() || !target.isValid()) {
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }
  QString reqscheme = request->url().scheme();
  if ((reqscheme == "https") && (target.scheme() != reqscheme)) {
    logger.log() << "Connection downgrade encountered during redirect";
    logger.log() << "Origin:" << request->url().toString();
    logger.log() << "Target:" << target.toString();
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }
  if (m_redirectlimit == 0) {
    logger.log() << "Connection redirection limit reached";
    authFailed(ErrorHandler::ErrorType::RemoteServiceError);
    return;
  }
  m_redirectlimit--;

  NetworkRequest* next =
      NetworkRequest::createForGetUrl(this, target.toString());
  connect(next, &NetworkRequest::requestFailed, this,
          &TaskPasswordAuth::netRequestFailed);
  connect(next, &NetworkRequest::requestHeaderReceived, this, callback);
}
