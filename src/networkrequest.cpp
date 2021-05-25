/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"
#include "captiveportal/captiveportal.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkmanager.h"
#include "settingsholder.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

// Timeout for the network requests.
constexpr uint32_t REQUEST_TIMEOUT_MSEC = 15000;

constexpr const char* IPINFO_URL_IPV4 = "https://%1/api/v1/vpn/ipinfo";
constexpr const char* IPINFO_URL_IPV6 = "https://[%1]/api/v1/vpn/ipinfo";

namespace {
Logger logger(LOG_NETWORKING, "NetworkRequest");
}

NetworkRequest::NetworkRequest(QObject* parent, int status)
    : QObject(parent), m_status(status) {
  MVPN_COUNT_CTOR(NetworkRequest);

  logger.log() << "Network request created";

#ifndef MVPN_WASM
  m_request.setRawHeader("User-Agent", NetworkManager::userAgent());
#endif

  // Let's use "glean-enabled" as an indicator for DNT/GPC too.
  if (!SettingsHolder::instance()->gleanEnabled()) {
    // Do-Not-Track:
    // https://datatracker.ietf.org/doc/html/draft-mayer-do-not-track-00
    m_request.setRawHeader("DNT", "1");
    // Global Privacy Control: https://globalprivacycontrol.github.io/gpc-spec/
    m_request.setRawHeader("Sec-GPC", "1");
  }

  m_timer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &NetworkRequest::timeout);
  connect(&m_timer, &QTimer::timeout, this, &QObject::deleteLater);

  NetworkManager::instance()->increaseNetworkRequestCount();
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
NetworkRequest* NetworkRequest::createForGetUrl(QObject* parent,
                                                const QString& url,
                                                int status) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, status);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    QObject* parent, const QString& pkceCodeSuccess,
    const QString& pkceCodeVerifier) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(Constants::API_URL);
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
NetworkRequest* NetworkRequest::createForDeviceCreation(
    QObject* parent, const QString& deviceName, const QString& pubKey) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 201);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/device");
  r->m_request.setUrl(url);

  QJsonObject obj;
  obj.insert("name", deviceName);
  obj.insert("pubkey", pubKey);

  QJsonDocument json;
  json.setObject(obj);

  r->postRequest(json.toJson(QJsonDocument::Compact));
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(QObject* parent,
                                                       const QString& pubKey) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 204);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QString url(Constants::API_URL);
  url.append("/api/v1/vpn/device/");
  url.append(QUrl::toPercentEncoding(pubKey));

  QUrl u(url);
  r->m_request.setUrl(QUrl(url));

#ifdef QT_DEBUG
  logger.log() << "Network starting" << r->m_request.url().toString();
#endif

  r->deleteRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForServers(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/servers");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForSurveyData(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/surveys");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForVersions(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/versions");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForAccount(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/account");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent,
                                                const QHostAddress& address) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    r->m_request.setUrl(QUrl(QString(IPINFO_URL_IPV6).arg(address.toString())));
  } else {
    Q_ASSERT(address.protocol() == QAbstractSocket::IPv4Protocol);
    r->m_request.setUrl(QUrl(QString(IPINFO_URL_IPV4).arg(address.toString())));
  }

  QUrl url(Constants::API_URL);
  r->m_request.setRawHeader("Host", url.host().toLocal8Bit());

  r->getRequest();

  // Only for this request, we ignore SSL errors, otherwise QT will try to
  // validate the SSL certificate using the hostname and not the Host-header
  // value.
  Q_ASSERT(r->m_reply);
  r->m_reply->ignoreSslErrors();

  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl& url, const QByteArray& host) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 0);

  r->m_request.setUrl(url);
  r->m_request.setRawHeader("Host", host);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(QObject* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/dns/detectportal");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForHeartbeat(QObject* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200);

  QUrl url(Constants::API_URL);
  url.setPath("/__heartbeat__");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

#ifdef MVPN_IOS
NetworkRequest* NetworkRequest::createForIOSProducts(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/products/ios");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForIOSPurchase(QObject* parent,
                                                     const QString& receipt) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 201);
  r->m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
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

void NetworkRequest::replyFinished() {
  Q_ASSERT(m_reply);
  Q_ASSERT(m_reply->isFinished());

  if (m_completed) {
    Q_ASSERT(!m_timer.isActive());
    return;
  }

  m_completed = true;
  m_timer.stop();

  int status = statusCode();

  logger.log() << "Network reply received - status:" << status
               << "- expected:" << m_status;

  QByteArray data = m_reply->readAll();

  if (m_reply->error() != QNetworkReply::NoError) {
    logger.log() << "Network error:" << m_reply->error()
                 << "status code:" << status << "- body:" << data;
    emit requestFailed(m_reply->error(), data);
    return;
  }

  // This is an extra check for succeeded requests (status code 200 vs 201, for
  // instance). The real network status check is done in the previous if-stmt.
  if (m_status && status != m_status) {
    logger.log() << "Status code unexpected - status code:" << status
                 << "- expected:" << m_status;
    emit requestFailed(QNetworkReply::ConnectionRefusedError, data);
    return;
  }

  emit requestCompleted(data);
}

void NetworkRequest::handleHeaderReceived() {
  logger.log() << "Network header received";
  emit requestHeaderReceived(this);
}

void NetworkRequest::timeout() {
  Q_ASSERT(m_reply);
  Q_ASSERT(!m_reply->isFinished());
  Q_ASSERT(!m_completed);

  m_completed = true;
  m_reply->abort();

  logger.log() << "Network request timeout";
  emit requestFailed(QNetworkReply::TimeoutError, QByteArray());
}

void NetworkRequest::getRequest() {
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->get(m_request));
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::deleteRequest() {
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->sendCustomRequest(m_request, "DELETE"));
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::postRequest(const QByteArray& body) {
  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->post(m_request, body));
  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::handleReply(QNetworkReply* reply) {
  Q_ASSERT(reply);
  Q_ASSERT(!m_reply);

  m_reply = reply;
  m_reply->setParent(this);

  connect(m_reply, &QNetworkReply::finished, this,
          &NetworkRequest::replyFinished);
  connect(m_reply, &QNetworkReply::metaDataChanged, this,
          &NetworkRequest::handleHeaderReceived);
  connect(m_reply, &QNetworkReply::finished, this, &QObject::deleteLater);
}

int NetworkRequest::statusCode() const {
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
    logger.log() << "INTERNAL ERROR! NetworkRequest::rawHeader called before "
                    "starting the request";
    return QByteArray();
  }

  return m_reply->rawHeader(headerName);
}

void NetworkRequest::abort() {
  if (!m_reply) {
    logger.log() << "INTERNAL ERROR! NetworkRequest::abort called before "
                    "starting the request";
    return;
  }

  m_reply->abort();
}
