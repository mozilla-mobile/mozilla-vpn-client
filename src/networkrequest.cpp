/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"
#include "captiveportal/captiveportal.h"
#include "constants.h"
#include "hawkauth.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkmanager.h"
#include "settingsholder.h"
#include "task.h"

#ifdef MVPN_WASM
#  include "platforms/wasm/wasmnetworkrequest.h"
#endif

#include <QDirIterator>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

// Timeout for the network requests.
constexpr uint32_t REQUEST_TIMEOUT_MSEC = 15000;
constexpr int REQUEST_MAX_REDIRECTS = 4;

constexpr const char* IPINFO_URL_IPV4 = "https://%1/api/v1/vpn/ipinfo";
constexpr const char* IPINFO_URL_IPV6 = "https://[%1]/api/v1/vpn/ipinfo";

namespace {
Logger logger(LOG_NETWORKING, "NetworkRequest");

#ifndef QT_NO_SSL
QList<QSslCertificate> s_intervention_certs;
#endif
}  // namespace

NetworkRequest::NetworkRequest(Task* parent, int status,
                               bool setAuthorizationHeader)
    : QObject(parent), m_expectedStatusCode(status) {
  MVPN_COUNT_CTOR(NetworkRequest);
  logger.debug() << "Network request created by" << parent->name();

  m_request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
  m_request.setRawHeader("User-Agent", NetworkManager::userAgent());
  m_request.setMaximumRedirectsAllowed(REQUEST_MAX_REDIRECTS);
  m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::SameOriginRedirectPolicy);

  // Let's use "glean-enabled" as an indicator for DNT/GPC too.
  if (!SettingsHolder::instance()->gleanEnabled()) {
    // Do-Not-Track:
    // https://datatracker.ietf.org/doc/html/draft-mayer-do-not-track-00
    m_request.setRawHeader("DNT", "1");
    // Global Privacy Control: https://globalprivacycontrol.github.io/gpc-spec/
    m_request.setRawHeader("Sec-GPC", "1");
  }
  m_request.setOriginatingObject(parent);

  if (setAuthorizationHeader) {
    if (SettingsHolder::instance()->token().isEmpty()) {
      logger.error() << "INVALID TOKEN! This network request is going to fail.";
      Q_ASSERT(false);
    }

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(
        SettingsHolder::instance()->token().toLocal8Bit());
    m_request.setRawHeader("Authorization", authorizationHeader);
  }

  m_timer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &NetworkRequest::timeout);
  connect(&m_timer, &QTimer::timeout, this, &NetworkRequest::maybeDeleteLater);

  NetworkManager::instance()->increaseNetworkRequestCount();

#ifndef QT_NO_SSL
  enableSSLIntervention();
#endif
}

NetworkRequest::~NetworkRequest() {
  MVPN_COUNT_DTOR(NetworkRequest);

  // During the shutdown, the QML NetworkManager can be released before the
  // deletion of the pending network requests.
  if (NetworkManager::exists()) {
    NetworkManager::instance()->decreaseNetworkRequestCount();
  }
}

// static
QString NetworkRequest::apiBaseUrl() {
  if (Constants::inProduction()) {
    return Constants::API_PRODUCTION_URL;
  }

  return Constants::getStagingServerAddress();
}

// static
NetworkRequest* NetworkRequest::createForGetUrl(Task* parent,
                                                const QString& url,
                                                int status) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, status, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");
  r->m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                            QNetworkRequest::NoLessSafeRedirectPolicy);

  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForGetHostAddress(
    Task* parent, const QString& url, const QHostAddress& address) {
  Q_ASSERT(parent);
  QUrl requestUrl(url);
  QString hostname = requestUrl.host();

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");
  r->m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                            QNetworkRequest::NoLessSafeRedirectPolicy);

  // Rewrite the request URL to use an explicit host address.
  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    requestUrl.setHost("[" + address.toString() + "]");
  } else {
    requestUrl.setHost(address.toString());
  }
  r->m_request.setUrl(requestUrl);
  r->m_request.setRawHeader("Host", hostname.toLocal8Bit());
  r->m_request.setPeerVerifyName(hostname);

  r->getRequest();
  return r;
}

// static
NetworkRequest* NetworkRequest::createForUploadData(Task* parent,
                                                    const QString& url,
                                                    QIODevice* uploadData) {
  Q_ASSERT(parent);
  Q_ASSERT(uploadData);
  QUrl requestUrl(url);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/x-www-form-urlencoded");
  r->m_request.setUrl(url);

  r->uploadDataRequest(uploadData);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForUploadDataHostAddress(
    Task* parent, const QString& url, QIODevice* uploadData,
    const QHostAddress& address) {
  Q_ASSERT(parent);
  Q_ASSERT(uploadData);
  QUrl requestUrl(url);
  QString hostname = requestUrl.host();

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/x-www-form-urlencoded");

  // Rewrite the request URL to use an explicit host address.
  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    requestUrl.setHost("[" + address.toString() + "]");
  } else {
    requestUrl.setHost(address.toString());
  }
  r->m_request.setUrl(requestUrl);
  r->m_request.setRawHeader("Host", hostname.toLocal8Bit());
  r->m_request.setPeerVerifyName(hostname);

  r->uploadDataRequest(uploadData);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    Task* parent, const QString& pkceCodeSuccess,
    const QString& pkceCodeVerifier) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(apiBaseUrl());
  url.setPath("/api/v2/vpn/login/verify");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("code", pkceCodeSuccess);
  obj.insert("code_verifier", pkceCodeVerifier);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForAdjustProxy(
    Task* parent, const QString& method, const QString& path,
    const QList<QPair<QString, QString>>& headers,
    const QString& queryParameters, const QString& bodyParameters,
    const QList<QString>& unknownParameters) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/adjust");
  r->m_request.setUrl(url);

  QJsonObject headersObj;
  for (const QPair<QString, QString>& header : headers) {
    headersObj.insert(header.first, header.second);
  }

  QJsonObject obj;
  obj.insert("method", method);
  obj.insert("path", path);
  obj.insert("headers", headersObj);
  obj.insert("queryParameters", queryParameters);
  obj.insert("bodyParameters", bodyParameters);

  QJsonArray unknownParametersArray;
  for (const QString& unknownParameter : unknownParameters) {
    unknownParametersArray.append(unknownParameter);
  }
  obj.insert("unknownParameters", unknownParametersArray);

  QJsonDocument json(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceCreation(
    Task* parent, const QString& deviceName, const QString& pubKey,
    const QString& deviceId) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 201, true);

  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/device");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("name", deviceName);
  obj.insert("unique_id", deviceId);
  obj.insert("pubkey", pubKey);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(Task* parent,
                                                       const QString& pubKey) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 204, true);

  QString url(apiBaseUrl());
  url.append("/api/v1/vpn/device/");
  url.append(QUrl::toPercentEncoding(pubKey));

  r->m_request.setUrl(QUrl(url));

#ifdef MVPN_DEBUG
  logger.debug() << "Network starting" << r->m_request.url().toString();
#endif

  r->deleteRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForServers(Task* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/servers");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForVersions(Task* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/versions");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForAccount(Task* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/account");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForGetSubscriptionDetails(Task* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/subscriptionDetails");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForIpInfo(Task* parent,
                                                const QHostAddress& address) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);

  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    r->m_request.setUrl(QUrl(QString(IPINFO_URL_IPV6).arg(address.toString())));
  } else {
    Q_ASSERT(address.protocol() == QAbstractSocket::IPv4Protocol);
    r->m_request.setUrl(QUrl(QString(IPINFO_URL_IPV4).arg(address.toString())));
  }

  QUrl url(apiBaseUrl());
  r->m_request.setRawHeader("Host", url.host().toLocal8Bit());
  r->m_request.setPeerVerifyName(url.host());

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    Task* parent, const QUrl& url, const QByteArray& host) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 0, false);

  r->m_request.setUrl(url);
  r->m_request.setRawHeader("Host", host);
  r->m_request.setPeerVerifyName(host);

  // This enables the QNetworkReply::redirected for every type of redirect.
  r->m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                            QNetworkRequest::UserVerifiedRedirectPolicy);
  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(Task* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200, true);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/dns/detectportal");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForHeartbeat(Task* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(apiBaseUrl());
  url.setPath("/__heartbeat__");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForFeedback(Task* parent,
                                                  const QString& feedbackText,
                                                  const QString& logs,
                                                  const qint8 rating,
                                                  const QString& category) {
  NetworkRequest* r = new NetworkRequest(parent, 201, true);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/feedback");
  r->m_request.setUrl(url);

  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("feedbackText", feedbackText);
  obj.insert("logs", logs);
  obj.insert("versionString", Env::versionString());
  obj.insert("platformVersion", QString(NetworkManager::osVersion()));
  obj.insert("rating", rating);
  obj.insert("category", category);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

NetworkRequest* NetworkRequest::createForSupportTicket(
    Task* parent, const QString& email, const QString& subject,
    const QString& issueText, const QString& logs, const QString& category) {
  bool isAuthenticated =
      MozillaVPN::instance()->userState() == MozillaVPN::UserAuthenticated;

  NetworkRequest* r = new NetworkRequest(parent, 201, isAuthenticated);

  QUrl url(apiBaseUrl());

  if (isAuthenticated) {
    url.setPath("/api/v1/vpn/createSupportTicket");
  } else {
    url.setPath("/api/v1/vpn/createGuestSupportTicket");
  }

  r->m_request.setUrl(url);

  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("email", email);
  obj.insert("logs", logs);
  obj.insert("versionString", Env::versionString());
  obj.insert("platformVersion", QString(NetworkManager::osVersion()));
  obj.insert("subject", subject);
  obj.insert("issueText", issueText);
  obj.insert("category", category);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForGetFeatureList(Task* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/featurelist");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountStatus(
    Task* parent, const QString& emailAddress) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/account/status");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("email", emailAddress);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountCreation(
    Task* parent, const QString& email, const QByteArray& authpw,
    const QString& fxaClientId, const QString& fxaDeviceId,
    const QString& fxaFlowId, double fxaFlowBeginTime) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/account/create");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("email", email);
  obj.insert("authPW", QString(authpw.toHex()));
  obj.insert("service", fxaClientId);
  obj.insert("verificationMethod", "email-otp");

  QJsonObject metrics;
  metrics.insert("deviceId", fxaDeviceId);
  metrics.insert("flowId", fxaFlowId);
  metrics.insert("flowBeginTime", fxaFlowBeginTime);
  obj.insert("metricsContext", metrics);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaLogin(
    Task* parent, const QString& email, const QByteArray& authpw,
    const QString& originalLoginEmail, const QString& unblockCode,
    const QString& fxaClientId, const QString& fxaDeviceId,
    const QString& fxaFlowId, double fxaFlowBeginTime) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/account/login");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("email", email);
  obj.insert("authPW", QString(authpw.toHex()));
  obj.insert("reason", "signin");
  obj.insert("service", fxaClientId);
  obj.insert("skipErrorCase", true);
  obj.insert("verificationMethod", "email-otp");

  if (!originalLoginEmail.isEmpty()) {
    obj.insert("originalLoginEmail", originalLoginEmail);
  }

  if (!unblockCode.isEmpty()) {
    obj.insert("unblockCode", unblockCode);
  }

  QJsonObject metrics;
  metrics.insert("deviceId", fxaDeviceId);
  metrics.insert("flowId", fxaFlowId);
  metrics.insert("flowBeginTime", fxaFlowBeginTime);
  obj.insert("metricsContext", metrics);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSendUnblockCode(
    Task* parent, const QString& emailAddress) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/account/login/send_unblock_code");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("email", emailAddress);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionVerifyByEmailCode(
    Task* parent, const QByteArray& sessionToken, const QString& code,
    const QString& fxaClientId, const QString& fxaScope) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/session/verify_code");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("code", code);
  obj.insert("service", fxaClientId);

  QJsonArray scopes;
  QStringList queryScopes = fxaScope.split(" ");
  foreach (const QString& s, queryScopes) {
    QString parsedScope;
    if (s.startsWith("http")) {
      parsedScope = QUrl::fromPercentEncoding(s.toUtf8());
    } else {
      parsedScope = s;
    }
    scopes.append(parsedScope);
  }
  obj.insert("scopes", scopes);

  QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionResendCode(
    Task* parent, const QByteArray& sessionToken) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/session/resend_code");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QByteArray payload =
      QJsonDocument(QJsonObject()).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionVerifyByTotpCode(
    Task* parent, const QByteArray& sessionToken, const QString& code,
    const QString& fxaClientId, const QString& fxaScope) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/session/verify/totp");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("code", code);
  obj.insert("service", fxaClientId);

  QJsonArray scopes;
  scopes.append(fxaScope);
  obj.insert("scopes", scopes);

  QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAuthz(
    Task* parent, const QByteArray& sessionToken, const QString& fxaClientId,
    const QString& fxaState, const QString& fxaScope,
    const QString& fxaAccessType) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/oauth/authorization");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("client_id", fxaClientId);
  obj.insert("state", fxaState);
  obj.insert("scope", fxaScope);
  obj.insert("access_type", fxaAccessType);

  QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}

#ifdef UNIT_TEST
// static
NetworkRequest* NetworkRequest::createForFxaTotpCreation(
    Task* parent, const QByteArray& sessionToken) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/totp/create");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QByteArray payload = "{}";

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}
#endif

// static
NetworkRequest* NetworkRequest::createForFxaAttachedClients(
    Task* parent, const QByteArray& sessionToken) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/account/attached_clients");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "GET", "").toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->getRequest();
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountDeletion(
    Task* parent, const QByteArray& sessionToken, const QString& emailAddress,
    const QByteArray& authpw) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/account/destroy");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QJsonObject obj;
  obj.insert("email", emailAddress);
  obj.insert("authPW", QString(authpw.toHex()));

  QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionDestroy(
    Task* parent, const QByteArray& sessionToken) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  QUrl url(Constants::fxaApiBaseUrl());
  url.setPath("/v1/session/destroy");
  r->m_request.setUrl(url);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QByteArray payload =
      QJsonDocument(QJsonObject()).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader = hawk.generate(r->m_request, "POST", payload).toUtf8();
  r->m_request.setRawHeader("Authorization", hawkHeader);

  r->postRequest(payload);
  return r;
}

NetworkRequest* NetworkRequest::createForProducts(Task* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);

  QUrl url(apiBaseUrl());
  url.setPath("/api/v3/vpn/products");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

#ifdef MVPN_IOS
NetworkRequest* NetworkRequest::createForIOSPurchase(Task* parent,
                                                     const QString& receipt) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 201, true);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/purchases/ios");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("receipt", QJsonValue(receipt));
  obj.insert("appId", "org.mozilla.ios.FirefoxVPN");

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}
#endif

#ifdef MVPN_ANDROID
NetworkRequest* NetworkRequest::createForAndroidPurchase(
    Task* parent, const QString& sku, const QString& purchaseToken) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/purchases/android");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("sku", sku);
  obj.insert("token", purchaseToken);

  QJsonDocument json;
  json.setObject(obj);

  logger.debug() << "Network request createForAndroidPurchase created"
                 << logger.sensitive(json.toJson(QJsonDocument::Compact));

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}
#endif

#ifdef MVPN_WASM
NetworkRequest* NetworkRequest::createForWasmPurchase(
    Task* parent, const QString& productId) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, true);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(apiBaseUrl());
  url.setPath("/api/v1/vpn/purchases/wasm");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("productId", productId);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}
#endif

void NetworkRequest::replyFinished() {
  Q_ASSERT(m_reply);
  Q_ASSERT(m_reply->isFinished());

  if (m_completed) {
    Q_ASSERT(!m_timer.isActive());
    return;
  }

#if QT_VERSION >= 0x060000 && QT_VERSION < 0x060400
  if (m_reply->error() == QNetworkReply::HostNotFoundError && isRedirect()) {
    QUrl brokenUrl = m_reply->url();

    if (brokenUrl.host().isEmpty() && !m_redirectedUrl.isEmpty()) {
      QUrl url = m_redirectedUrl.resolved(brokenUrl);

#  ifdef MVPN_DEBUG
      // See https://bugreports.qt.io/browse/QTBUG-100651
      logger.debug()
          << "QT6 redirect bug! The current URL is broken because it's not "
             "resolved using the latest HTTP redirection as base-URL";
      logger.debug() << "Broken URL:" << brokenUrl.toString();
      logger.debug() << "Latest redirected URL:" << m_redirectedUrl.toString();
      logger.debug() << "Final URL:" << url.toString();
#  endif

      m_request = QNetworkRequest(url);
      m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/json");
      m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);

      m_reply = nullptr;
      m_timer.stop();
      getRequest();
      return;
    }
  }
#endif

  int status = statusCode();

  QString expect =
      m_expectedStatusCode ? QString::number(m_expectedStatusCode) : "any";
  logger.debug() << "Network reply received - status:" << status
                 << "- expected:" << expect;

  QByteArray data = m_reply->readAll();
  processData(m_reply->error(), m_reply->errorString(), status, data);
}

void NetworkRequest::processData(QNetworkReply::NetworkError error,
                                 const QString& errorString, int status,
                                 const QByteArray& data) {
  m_completed = true;
  m_timer.stop();

#ifdef MVPN_WASM
  m_finalStatusCode = status;
#endif

  if (error != QNetworkReply::NoError) {
    QUrl::FormattingOptions options = QUrl::RemoveQuery | QUrl::RemoveUserInfo;
    logger.error() << "Network error:" << errorString
                   << "status code:" << status
                   << "- body:" << logger.sensitive(data);
    logger.error() << "Failed to access:" << m_request.url().toString(options);
    emit requestFailed(error, data);
    return;
  }

  // This is an extra check for succeeded requests (status code 200 vs 201, for
  // instance). The real network status check is done in the previous if-stmt.
  if (m_expectedStatusCode && status != m_expectedStatusCode) {
    logger.error() << "Status code unexpected - status code:" << status
                   << "- expected:" << m_expectedStatusCode;
    emit requestFailed(QNetworkReply::ConnectionRefusedError, data);
    return;
  }

  emit requestCompleted(data);
}

bool NetworkRequest::isRedirect() const {
  int status = statusCode();
  return status >= 300 && status < 400;
}

void NetworkRequest::handleHeaderReceived() {
  // Suppress this signal if a redirect is about to happen.
  int policy =
      m_request.attribute(QNetworkRequest::RedirectPolicyAttribute).toInt();
  if (isRedirect() && (policy != QNetworkRequest::ManualRedirectPolicy)) {
    return;
  }

  logger.debug() << "Network header received";
  emit requestHeaderReceived(this);
}

void NetworkRequest::handleRedirect(const QUrl& redirectUrl) {
#if QT_VERSION >= 0x060000 && QT_VERSION < 0x060400
  if (redirectUrl.host().isEmpty()) {
#  ifdef MVPN_DEBUG
    // See https://bugreports.qt.io/browse/QTBUG-100651
    logger.debug()
        << "QT6 redirect bug! The redirected URL is broken because it's not "
           "resolved using the previous HTTP redirection as base-URL";
    logger.debug() << "Broken URL:" << redirectUrl.toString();
    logger.debug() << "Latest redirected URL:" << m_redirectedUrl.toString();
#  endif

    if (m_redirectedUrl.isEmpty()) {
      m_redirectedUrl = url().resolved(redirectUrl);
    } else {
      m_redirectedUrl = m_redirectedUrl.resolved(redirectUrl);
    }
  } else {
    m_redirectedUrl = redirectUrl;
  }
  emit requestRedirected(this, m_redirectedUrl);
#else
  emit requestRedirected(this, redirectUrl);
#endif
}

void NetworkRequest::timeout() {
#ifndef MVPN_WASM
  Q_ASSERT(m_reply);
  Q_ASSERT(!m_reply->isFinished());
#endif
  Q_ASSERT(!m_completed);

  m_completed = true;

  if (m_reply) {
    m_reply->abort();
  }

  logger.error() << "Network request timeout";
  emit requestFailed(QNetworkReply::TimeoutError, QByteArray());
}

void NetworkRequest::getRequest() {
#ifdef MVPN_WASM
  WasmNetworkRequest::getRequest(this);
#else
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->get(m_request));
#endif
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::deleteRequest() {
#ifdef MVPN_WASM
  WasmNetworkRequest::deleteRequest(this);
#else
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->sendCustomRequest(m_request, "DELETE"));
#endif
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::postRequest(const QByteArray& body) {
#ifdef MVPN_WASM
  WasmNetworkRequest::postRequest(this, body);
#else
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->post(m_request, body));
#endif
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::uploadDataRequest(QIODevice* data) {
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->post(m_request, data));
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::handleReply(QNetworkReply* reply) {
  Q_ASSERT(reply);
  Q_ASSERT(!m_reply);

  m_reply = reply;
  m_reply->setParent(this);

  connect(m_reply, &QNetworkReply::finished, this,
          &NetworkRequest::replyFinished);
#ifndef QT_NO_SSL
  connect(m_reply, &QNetworkReply::sslErrors, this, &NetworkRequest::sslErrors);
#endif
  connect(m_reply, &QNetworkReply::metaDataChanged, this,
          &NetworkRequest::handleHeaderReceived);
  connect(m_reply, &QNetworkReply::redirected, this,
          &NetworkRequest::handleRedirect);
  connect(m_reply, &QNetworkReply::finished, this,
          &NetworkRequest::maybeDeleteLater);
  connect(m_reply, &QNetworkReply::downloadProgress, this,
          [&](qint64 bytesReceived, qint64 bytesTotal) {
            emit requestUpdated(bytesReceived, bytesTotal, m_reply);
          });
  connect(m_reply, &QNetworkReply::uploadProgress, this,
          [&](qint64 bytesSent, qint64 bytesTotal) {
            uploadProgressed(bytesSent, bytesTotal, m_reply);
          });
}

void NetworkRequest::maybeDeleteLater() {
  if (m_reply && m_reply->isFinished()) {
    deleteLater();
  }
}

int NetworkRequest::statusCode() const {
#ifdef MVPN_WASM
  return m_finalStatusCode;
#endif

  Q_ASSERT(m_reply);

  QVariant statusCode =
      m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (!statusCode.isValid()) {
    return 0;
  }

  return statusCode.toInt();
}

void NetworkRequest::disableTimeout() { m_timer.stop(); }

QByteArray NetworkRequest::rawHeader(const QByteArray& headerName) const {
  if (!m_reply) {
    logger.error() << "INTERNAL ERROR! NetworkRequest::rawHeader called before "
                      "starting the request";
    return QByteArray();
  }

  return m_reply->rawHeader(headerName);
}

void NetworkRequest::abort() {
  m_aborted = true;

  if (!m_reply) {
    logger.error() << "INTERNAL ERROR! NetworkRequest::abort called before "
                      "starting the request";
    return;
  }

  m_reply->abort();
}

#ifndef QT_NO_SSL
bool NetworkRequest::checkSubjectName(const QSslCertificate& cert) {
  QString hostname = QString(m_request.rawHeader("Host"));
  if (hostname.isEmpty()) {
    Q_ASSERT(m_reply);
    hostname = m_reply->url().host();
  }

  // Check if there is a match in the subject common name.
  QStringList commonNames = cert.subjectInfo(QSslCertificate::CommonName);
  if (commonNames.contains(hostname)) {
    logger.debug() << "Found commonName match for" << hostname;
    return true;
  }

  // Check there is a match amongst the subject alternative names.
  QStringList altNames = cert.subjectAlternativeNames().values(QSsl::DnsEntry);
  for (const QString& pattern : altNames) {
    QRegularExpression re(
        QRegularExpression::wildcardToRegularExpression(pattern));
    if (re.match(hostname).hasMatch()) {
      logger.debug() << "Found subjectAltName match for" << hostname;
      return true;
    }
  }

  // If we get this far, then the certificate has no matching subject name.
  return false;
}

void NetworkRequest::sslErrors(const QList<QSslError>& errors) {
  if (!m_reply) {
    return;
  }

  // Manually check for a hostname match in case we set a raw Host header.
  if ((errors.count() == 1) && m_request.hasRawHeader("Host") &&
      (errors[0].error() == QSslError::HostNameMismatch) &&
      checkSubjectName(errors[0].certificate())) {
    m_reply->ignoreSslErrors(errors);
    return;
  }

  logger.error() << "SSL Error on" << m_reply->url().host();
  for (const auto& error : errors) {
    logger.error() << error.errorString();
    auto cert = error.certificate();
    if (!cert.isNull()) {
      logger.info() << "Related Cert:";
      logger.info() << cert.toText();
    }
  }
}

void NetworkRequest::enableSSLIntervention() {
  if (s_intervention_certs.isEmpty()) {
    s_intervention_certs = QSslConfiguration::systemCaCertificates();
    QDirIterator certFolder(":/certs");
    while (certFolder.hasNext()) {
      QFile f(certFolder.next());
      if (!f.open(QIODevice::ReadOnly)) {
        continue;
      }
      QSslCertificate cert(&f, QSsl::Pem);
      if (!cert.isNull()) {
        logger.info() << "Imported cert from:" << cert.issuerDisplayName();
        s_intervention_certs.append(cert);
      } else {
        logger.error() << "Failed to import cert -" << f.fileName();
      }
    }
  }
  if (s_intervention_certs.isEmpty()) {
    return;
  }
  auto conf = QSslConfiguration::defaultConfiguration();
  conf.addCaCertificates(s_intervention_certs);
  m_request.setSslConfiguration(conf);
}
#endif
