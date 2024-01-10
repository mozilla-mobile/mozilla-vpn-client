/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"

#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

#include "logging/logger.h"
#include "networkmanager.h"
#include "settings/settingsholder.h"
#include "task.h"
#include "utilities/leakdetector.h"

// Timeout for the network requests.
constexpr uint32_t REQUEST_TIMEOUT_MSEC = 15000;
constexpr int REQUEST_MAX_REDIRECTS = 4;

namespace {
Logger logger("NetworkRequest");

#ifndef QT_NO_SSL
QList<QSslCertificate> s_intervention_certs;
#endif

std::function<bool(NetworkRequest*)> s_deleteResourceCallback = nullptr;
std::function<bool(NetworkRequest*)> s_getResourceCallback = nullptr;
std::function<bool(NetworkRequest*, const QByteArray&)> s_postResourceCallback =
    nullptr;
std::function<bool(NetworkRequest*, QIODevice*)>
    s_postResourceIODeviceCallback = nullptr;

}  // namespace

// static
void NetworkRequest::setRequestHandler(
    std::function<bool(NetworkRequest*)>&& deleteResourceCallback,
    std::function<bool(NetworkRequest*)>&& getResourceCallback,
    std::function<bool(NetworkRequest*, const QByteArray&)>&&
        postResourceCallback,
    std::function<bool(NetworkRequest*, QIODevice*)>&&
        postResourceIODeviceCallback) {
  s_deleteResourceCallback = std::move(deleteResourceCallback);
  s_getResourceCallback = std::move(getResourceCallback);
  s_postResourceCallback = std::move(postResourceCallback);
  s_postResourceIODeviceCallback = std::move(postResourceIODeviceCallback);
}

NetworkRequest::NetworkRequest(Task* parent, int status)
    : QObject(parent), m_expectedStatusCode(status) {
  MZ_COUNT_CTOR(NetworkRequest);
  logger.debug() << "Network request created by" << parent->name();

  m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
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

  m_timer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &NetworkRequest::timeout);
  connect(&m_timer, &QTimer::timeout, this, &NetworkRequest::maybeDeleteLater);

  NetworkManager::instance()->increaseNetworkRequestCount();

#ifndef QT_NO_SSL
  enableSSLIntervention();
#endif
}

NetworkRequest::~NetworkRequest() {
  MZ_COUNT_DTOR(NetworkRequest);

  // During the shutdown, the QML NetworkManager can be released before the
  // deletion of the pending network requests.
  if (NetworkManager::exists()) {
    NetworkManager::instance()->decreaseNetworkRequestCount();
  }
}

void NetworkRequest::auth(const QByteArray& authorizationHeader) {
  m_request.setRawHeader("Authorization", authorizationHeader);
}

void NetworkRequest::get(const QUrl& url) {
  m_request.setUrl(url);
  getResource();
}

void NetworkRequest::post(const QUrl& url, QIODevice* uploadData) {
  m_request.setUrl(url);

  if (s_postResourceIODeviceCallback &&
      s_postResourceIODeviceCallback(this, uploadData)) {
    return;
  }

  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->post(m_request, uploadData));

  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::post(const QUrl& url, const QJsonObject& obj) {
  post(url, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void NetworkRequest::post(const QUrl& url, const QByteArray& body) {
  m_request.setUrl(url);

  if (s_postResourceCallback && s_postResourceCallback(this, body)) {
    return;
  }

  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->post(m_request, body));

  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::deleteResource(const QUrl& url) {
  m_request.setUrl(url);

  if (s_deleteResourceCallback && s_deleteResourceCallback(this)) {
    return;
  }

  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->deleteResource(m_request));

  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::replyFinished() {
  Q_ASSERT(m_reply);
  Q_ASSERT(m_reply->isFinished());

  if (m_completed) {
    Q_ASSERT(!m_timer.isActive());
    return;
  }

#if QT_VERSION < 0x060400
  if (m_reply->error() == QNetworkReply::HostNotFoundError && isRedirect()) {
    QUrl brokenUrl = m_reply->url();

    if (brokenUrl.host().isEmpty() && !m_redirectedUrl.isEmpty()) {
      QUrl url = m_redirectedUrl.resolved(brokenUrl);

#  ifdef MZ_DEBUG
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
      getResource();
      return;
    }
  }
#endif

  int status = statusCode();

  QString expect =
      m_expectedStatusCode ? QString::number(m_expectedStatusCode) : "any";
  logger.debug() << "Network reply received - status:" << status
                 << "- expected:" << expect;

  m_replyData.append(m_reply->readAll());
  processData(m_reply->error(), m_reply->errorString(), status, m_replyData);
}

void NetworkRequest::processData(QNetworkReply::NetworkError error,
                                 const QString& errorString, int status,
                                 const QByteArray& data) {
  m_completed = true;
  m_timer.stop();

#ifdef MZ_WASM
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

qint64 NetworkRequest::discardData() {
  qint64 bytes = m_replyData.count();
  if (m_reply != nullptr) {
    bytes += m_reply->skip(m_reply->bytesAvailable());
  }
  m_replyData.clear();
  return bytes;
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
#if QT_VERSION < 0x060400
  if (redirectUrl.host().isEmpty()) {
#  ifdef MZ_DEBUG
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
  Q_ASSERT(!m_completed);

  m_completed = true;

  if (m_reply) {
    m_reply->abort();
  }

  logger.error() << "Network request timeout";
  emit requestFailed(QNetworkReply::TimeoutError, QByteArray());
}

void NetworkRequest::getResource() {
  if (s_getResourceCallback && s_getResourceCallback(this)) {
    return;
  }

  QNetworkAccessManager* manager =
      NetworkManager::instance()->networkAccessManager();
  handleReply(manager->get(m_request));

  m_timer.start(REQUEST_TIMEOUT_MSEC);
}

void NetworkRequest::handleReply(QNetworkReply* reply) {
  Q_ASSERT(reply);
  Q_ASSERT(!m_reply);

  m_reply = reply;
  m_reply->setParent(this);

  connect(m_reply, &QNetworkReply::finished, this,
          &NetworkRequest::replyFinished);

  m_replyData.clear();
  connect(m_reply, &QIODevice::readyRead, this,
          [&]() { m_replyData.append(m_reply->readAll()); });

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
#ifdef MZ_WASM
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
