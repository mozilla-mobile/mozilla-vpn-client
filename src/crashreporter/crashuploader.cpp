/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashuploader.h"
#include "constants.h"
#include "qmlengineholder.h"
#include <QApplication>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QNetworkReply>
#include "logger.h"

using namespace std;

constexpr auto MAX_RETRIES = 3;
constexpr auto BOUNDARY = "--------------------XYsowmfWDDGdos";

namespace {
Logger logger(LOG_CRASHREPORTER, "CrashUploader");
}

CrashUploader::CrashUploader(QObject* parent) : QObject(parent) {
  m_network = QmlEngineHolder::instance()->networkAccessManager();

  connect(m_network, &QNetworkAccessManager::finished, this,
          &CrashUploader::requestComplete);
}

void CrashUploader::startUploads(QStringList files) {
  m_files = files;
  nextUpload();
}

void CrashUploader::nextUpload() {
  if (!m_files.empty()) {
    m_currentFile = m_files.first();
    m_files.pop_front();
    m_retries = 0;
    startRequest(m_currentFile);
  } else {
    emit uploadsComplete();
  }
}

void CrashUploader::startRequest(const QString& file) {
  logger.debug() << "Starting upload of " << file;
  QFile dump(file);
  if (!dump.open(QIODevice::ReadOnly)) {
    // fail and try the next one
    logger.error() << "Unable to open file: " << file;
    nextUpload();
    return;
  }
  m_retries++;
  if (m_retries > MAX_RETRIES) {
    nextUpload();
    return;
  }
  auto fileName = QFileInfo(file).fileName();
  auto dumpContent = dump.readAll();

  auto multipart = new QHttpMultiPart(this);
  multipart->setBoundary(QString(BOUNDARY).toLocal8Bit());
  QHttpPart formPart;
  auto contentDisp = QString(
                         "form-data; name=\"upload_file_minidump\"; "
                         "filename=\"%1\"")
                         .arg(fileName);
  formPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     QVariant(contentDisp));
  formPart.setHeader(QNetworkRequest::ContentTypeHeader,
                     "application/octet-stream");
  formPart.setBody(dumpContent);
  QHttpPart namePart;
  namePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     "form-data; name=\"ProductName\"");
  namePart.setBody(QString("MozillaVPN").toLocal8Bit());

  QHttpPart versionPart;
  versionPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"Version\"");
  versionPart.setBody(APP_VERSION);
  multipart->append(versionPart);
  multipart->append(namePart);
  multipart->append(formPart);

  auto urlStr = Constants::inProduction() ? Constants::CRASH_STAGING_URL
                                          : Constants::CRASH_PRODUCTION_URL;
#ifdef MVPN_DEBUG
  urlStr = Constants::CRASH_STAGING_URL;
#endif  // MVPN_DEBUG

  logger.debug() << "Uploading to : " << urlStr;
  QUrl url(urlStr);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader, "mozillaVPN");
  // Qt changes the content type automatically to multipart/mixed.  Socorro
  // doesn't support that, so we have to set it ourselves and create a custom
  // boundary.
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QString("multipart/form-data; boundary=%1").arg(BOUNDARY));
  auto reply = m_network->post(request, multipart);

  connect(reply, &QNetworkReply::sslErrors, [](QList<QSslError> errors) {
    logger.error() << "SSL Errors: ";
    for (auto err : errors) {
      logger.error() << err.errorString();
    }
  });
}

void CrashUploader::requestComplete(QNetworkReply* reply) {
#ifdef MVPN_DEBUG
  dumpResponse(reply);
#endif  // MVPN_DEBUG

  if (reply->error() != QNetworkReply::NoError) {
    logger.error() << "Upload failed for " << m_currentFile
                   << " error: " << reply->errorString()
                   << " code: " << reply->error();
    startRequest(m_currentFile);
  } else {
    logger.debug() << "Completed upload of " << m_currentFile;
    m_retries = 0;

    nextUpload();
  }
  reply->deleteLater();
}

void CrashUploader::dumpResponse(QNetworkReply* reply) {
  logger.debug() << reply->errorString();
  auto response = reply->readAll();
  auto headers = reply->rawHeaderList();
  logger.debug() << "Reply headers";
  for (auto header : headers) {
    logger.debug() << "Header: " << QString::fromLocal8Bit(header) << " = "
                   << QString::fromLocal8Bit(reply->rawHeader(header));
  }
  QString respBody(response);
  logger.debug() << "Response Body: " << respBody;
}
