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

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

// Timeout for the network requests.
constexpr uint32_t REQUEST_TIMEOUT_MSEC = 15000;

namespace {
Logger logger(LOG_NETWORKING, "NetworkRequest");
}

NetworkRequest::NetworkRequest(QObject* parent, int status)
    : QObject(parent), m_status(status) {
  MVPN_COUNT_CTOR(NetworkRequest);

  logger.log() << "Network request created";

  m_request.setRawHeader("User-Agent", NetworkManager::userAgent());
  m_timer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &NetworkRequest::timeout);
  connect(&m_timer, &QTimer::timeout, this, &QObject::deleteLater);
}

NetworkRequest::~NetworkRequest() { MVPN_COUNT_DTOR(NetworkRequest); }

// static
NetworkRequest* NetworkRequest::createForUrl(QObject* parent,
                                             const QString& url) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
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

NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  r->m_request.setRawHeader("Authorization", authorizationHeader);

  QUrl url(Constants::API_URL);
  url.setPath("/api/v1/vpn/ipinfo");
  r->m_request.setUrl(url);

  r->getRequest();
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl& url, const QByteArray& host) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

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

  if (m_timeout) {
    Q_ASSERT(!m_timer.isActive());
    return;
  }

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

  if (m_status && status != m_status) {
    logger.log() << "Status code unexpected - status code:" << status
                 << "- expected:" << m_status;
    emit requestFailed(m_reply->error(), data);
    return;
  }

  emit requestCompleted(m_reply, data);
}

void NetworkRequest::timeout() {
  Q_ASSERT(m_reply);
  Q_ASSERT(!m_reply->isFinished());

  m_timeout = true;
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
  connect(m_reply, &QNetworkReply::finished, this, &QObject::deleteLater);
}

int NetworkRequest::statusCode() const {
  Q_ASSERT(m_reply);
  Q_ASSERT(m_reply->isFinished());

  QVariant statusCode =
      m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (!statusCode.isValid()) {
    return 0;
  }

  return statusCode.toInt();
}

void NetworkRequest::disableTimeout() {
  m_timer.stop();
}
