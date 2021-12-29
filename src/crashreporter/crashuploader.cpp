/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashuploader.h"
#include "constants.h"
#include "qmlengineholder.h"
#include <iostream>
#include <QApplication>
#include <QHttpMultiPart>
#include <QFile>
#include <QByteArray>
#include <QNetworkReply>

using namespace std;

constexpr auto MAX_RETRIES = 1;
constexpr auto BOUNDARY = "--------------------XYsowmfWDDGdos";

CrashUploader::CrashUploader(QObject* parent) : QObject(parent) {
  m_network = QmlEngineHolder::instance()->networkAccessManager();

  connect(m_network, &QNetworkAccessManager::finished, this,
          &CrashUploader::requestComplete);
}

void CrashUploader::startUploads(QStringList files) {
  m_files = files;
  m_attempted = m_files.size();
  nextUpload();
}

void CrashUploader::nextUpload() {
  if (!m_files.empty()) {
    m_currentFile = m_files.first();
    m_files.pop_front();
    m_retries = 0;
    startRequest(m_currentFile);
  } else {
    emit uploadsComplete(m_attempted, m_completed);
  }
}

void CrashUploader::startRequest(const QString& file) {
  cout << "Starting upload of " << file.toStdString() << endl;
  QFile dump(file);
  if (!dump.open(QIODevice::ReadOnly)) {
    // fail and try the next one
    cout << "Unable to open file: " << file.toStdString() << endl;
    nextUpload();
    return;
  }
  m_retries++;
  if (m_retries > MAX_RETRIES) {
    nextUpload();
    return;
  }
  QString fileName = file.split("\\").last();
  QByteArray dumpContent = dump.readAll();

  QHttpMultiPart* multipart = new QHttpMultiPart(this);
  multipart->setBoundary(QString(BOUNDARY).toLocal8Bit());
  QHttpPart formPart;
  QString contentDisp = QString(
                            "form-data; name=\"upload_file_minidump\"; "
                            "filename=\"%1\"")
                            .arg(fileName);
  formPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     QVariant(contentDisp));
  formPart.setHeader(QNetworkRequest::ContentLengthHeader,
                     QVariant(dumpContent.size()));
  formPart.setBody(dumpContent);
  QHttpPart namePart;
  namePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     "form-data; name=\"ProductName\"");
  namePart.setBody(QString("MozillaVPN").toLocal8Bit());

  QHttpPart versionPart;
  versionPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"Version\"");
  versionPart.setBody(APP_VERSION);
  multipart->append(namePart);
  multipart->append(formPart);
  multipart->append(versionPart);
  // multipart->append(json);
  cout << "Uploading to : " << Constants::CRASH_STAGING_URL << endl;
  QUrl url(Constants::CRASH_STAGING_URL);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader, "mozillaVPN");
  // Qt changes the content type automatically to multipart/mixed.  Socorro
  // doesn't support that, so we have to set it ourselves and create a custom
  // bounday.
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QString("multipart/form-data; boundary=%1").arg(BOUNDARY));
  auto reply = m_network->post(request, multipart);
  connect(reply, &QNetworkReply::errorOccurred, this,
          &CrashUploader::requestFailed);
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void CrashUploader::requestComplete(QNetworkReply* reply) {
  std::cout << "Request Complete" << endl;
  if (reply->error() != QNetworkReply::NoError) {
    std::cout << "Upload failed for " << m_currentFile.toStdString() << endl;
    std::cout << reply->errorString().toStdString() << std::endl;
    auto response = reply->readAll();
    auto headers = reply->rawHeaderList();
    cout << "Reply headers" << endl;
    for (auto header : headers) {
      cout << "Header: " << QString::fromLocal8Bit(header).toStdString()
           << " = "
           << QString::fromLocal8Bit(reply->rawHeader(header)).toStdString()
           << endl;
    }
    QString respBody(response);
    std::cout << respBody.toStdString();
    // maybe retry...
    cout << "Request Headers" << endl;
    headers = reply->request().rawHeaderList();
    for (auto header : headers) {
      cout << "Header: " << QString::fromLocal8Bit(header).toStdString()
           << " = "
           << QString::fromLocal8Bit(reply->request().rawHeader(header))
                  .toStdString()
           << endl;
    }
    startRequest(m_currentFile);
  } else {
    cout << "Completed upload of " << m_currentFile.toStdString();
    m_retries = 0;
    m_completed++;
    nextUpload();
  }
  reply->deleteLater();
}

void CrashUploader::requestFailed(QNetworkReply::NetworkError error) {
  std::cout << "Upload failed for " << m_currentFile.toStdString() << " Error "
            << error << endl;
}
