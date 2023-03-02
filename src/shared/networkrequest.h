/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKREQUEST_H
#define NETWORKREQUEST_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <functional>

class QHostAddress;
class QNetworkAccessManager;
#ifndef QT_NO_SSL
class QSslCertificate;
#endif
class Task;

class NetworkRequest final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetworkRequest)

 public:
  // This object deletes itself at the end of the operation.

  NetworkRequest(Task* parent, int status = 0);
  ~NetworkRequest();

  static void setRequestHandler(
      std::function<bool(NetworkRequest*)>&& deleteResourceCallback,
      std::function<bool(NetworkRequest*)>&& getResourceCallback,
      std::function<bool(NetworkRequest*, const QByteArray&)>&&
          postResourceCallback,
      std::function<bool(NetworkRequest*, QIODevice*)>&&
          postResourceIODeviceCallback);

  void get(const QUrl& url);

  void post(const QUrl& url, QIODevice* uploadData);
  void post(const QUrl& url, const QJsonObject& obj);
  void post(const QUrl& url, const QByteArray& body);

  void deleteResource(const QUrl& url);

  QNetworkRequest& requestInternal() { return m_request; }

  void auth(const QByteArray& authorizationHeader);

  void disableTimeout();

  int statusCode() const;

  QByteArray rawHeader(const QByteArray& headerName) const;
  QUrl url() const { return m_reply ? m_reply->url() : m_request.url(); }

  void abort();
  bool isAborted() const { return m_aborted; }

  void processData(QNetworkReply::NetworkError error,
                   const QString& errorString, int status,
                   const QByteArray& data);

 private:
  void getResource();

  void handleReply(QNetworkReply* reply);
  void handleHeaderReceived();
  void handleRedirect(const QUrl& url);

#ifndef QT_NO_SSL
  bool checkSubjectName(const QSslCertificate& cert);
#endif

  bool isRedirect() const;

  void maybeDeleteLater();

 private slots:
  void replyFinished();
  void timeout();

#ifndef QT_NO_SSL
  void sslErrors(const QList<QSslError>& errors);
#endif

 signals:
  void requestHeaderReceived(NetworkRequest* request);
  void requestFailed(QNetworkReply::NetworkError error, const QByteArray& data);
  void requestRedirected(NetworkRequest* request, const QUrl& url);
  void requestCompleted(const QByteArray& data);
  void requestUpdated(qint64 bytesReceived, qint64 bytesTotal,
                      QNetworkReply* reply);
  void uploadProgressed(qint64 bytesReceived, qint64 bytesTotal,
                        QNetworkReply* reply);

 private:
  QNetworkRequest m_request;
  QTimer m_timer;

  QUrl m_redirectedUrl;

#ifndef QT_NO_SSL
  /**
   * @brief this is a workaround against a buggy old version of Firefox with an
   * expired SRG Root X1 cert
   */
  void enableSSLIntervention();
#endif

  QNetworkReply* m_reply = nullptr;
  QByteArray m_replyData;
  int m_expectedStatusCode = 0;
#ifdef MZ_WASM
  // In wasm network request, m_reply is null. So we need to store the "status
  // code" in a variable member.
  int m_finalStatusCode = 0;
#endif

  bool m_completed = false;
  bool m_aborted = false;
};

#endif  // NETWORKREQUEST_H
