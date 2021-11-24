/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKREQUEST_H
#define NETWORKREQUEST_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>

class QHostAddress;
class QNetworkAccessManager;
class QurlQuery;

class NetworkRequest final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetworkRequest)

 public:
  ~NetworkRequest();

  // This object deletes itself at the end of the operation.

  static NetworkRequest* createForGetUrl(QObject* parent, const QString& url,
                                         int status = 0);

  static NetworkRequest* createForAuthenticationVerification(
      QObject* parent, const QString& pkceCodeSuccess,
      const QString& pkceCodeVerifier);

  static NetworkRequest* createForAdjustProxy(
      QObject* parent, const QString& method, const QString& path,
      const QList<QPair<QString, QString>>& headers,
      const QString& queryParameters, const QString& bodyParameters,
      const QList<QString>& unknownParameters);

  static NetworkRequest* createForDeviceCreation(QObject* parent,
                                                 const QString& deviceName,
                                                 const QString& pubKey,
                                                 const QString& deviceId);

  static NetworkRequest* createForDeviceRemoval(QObject* parent,
                                                const QString& pubKey);

  static NetworkRequest* createForServers(QObject* parent);

  static NetworkRequest* createForAccount(QObject* parent);

  static NetworkRequest* createForVersions(QObject* parent);

  static NetworkRequest* createForIpInfo(QObject* parent,
                                         const QHostAddress& address);

  static NetworkRequest* createForCaptivePortalDetection(
      QObject* parent, const QUrl& url, const QByteArray& host);

  static NetworkRequest* createForCaptivePortalLookup(QObject* parent);

  static NetworkRequest* createForHeartbeat(QObject* parent);

  static NetworkRequest* createForSurveyData(QObject* parent);

  static NetworkRequest* createForFeedback(QObject* parent,
                                           const QString& feedbackText,
                                           const QString& logs,
                                           const qint8 rating,
                                           const QString& category);

  static NetworkRequest* createForSupportTicket(
      QObject* parent, const QString& email, const QString& subject,
      const QString& issueText, const QString& logs, const QString& category);

  static NetworkRequest* createForGetFeatureList(QObject* parent);

  static NetworkRequest* createForFxaAccountStatus(QObject* parent,
                                                   const QString& emailAddress);

  static NetworkRequest* createForFxaAccountCreation(QObject* parent,
                                                     const QString& email,
                                                     const QByteArray& authpw,
                                                     const QUrlQuery& query);

  static NetworkRequest* createForFxaLogin(QObject* parent,
                                           const QString& email,
                                           const QByteArray& authpw,
                                           const QString& unblockCode,
                                           const QUrlQuery& query);

  static NetworkRequest* createForFxaSendUnblockCode(
      QObject* parent, const QString& emailAddress);

  static NetworkRequest* createForFxaSessionVerifyByEmailCode(
      QObject* parent, const QByteArray& sessionToken, const QString& code,
      const QUrlQuery& query);

  static NetworkRequest* createForFxaSessionVerifyByTotpCode(
      QObject* parent, const QByteArray& sessionToken, const QString& code,
      const QUrlQuery& query);

  static NetworkRequest* createForFxaSessionResendCode(
      QObject* parent, const QByteArray& sessionToken);

  static NetworkRequest* createForFxaAuthz(QObject* parent,
                                           const QByteArray& sessionToken,
                                           const QUrlQuery& query);

#ifdef UNIT_TEST
  static NetworkRequest* createForFxaTotpCreation(
      QObject* parent, const QByteArray& sessionToken);
#endif

  static NetworkRequest* createForFxaSessionDestroy(
      QObject* parent, const QByteArray& sessionToken);

  static NetworkRequest* createForProducts(QObject* parent);

#ifdef MVPN_IOS
  static NetworkRequest* createForIOSPurchase(QObject* parent,
                                              const QString& receipt);
#endif
#ifdef MVPN_ANDROID
  static NetworkRequest* createForAndroidPurchase(QObject* parent,
                                                  const QString& sku,
                                                  const QString& purchaseToken);
#endif

  void disableTimeout();

  int statusCode() const;

  QByteArray rawHeader(const QByteArray& headerName) const;
  QUrl url() const { return m_reply ? m_reply->url() : m_request.url(); }

  void abort();

  static QString apiBaseUrl();

 private:
  NetworkRequest(QObject* parent, int status, bool setAuthorizationHeader);

  void deleteRequest();
  void getRequest();
  void postRequest(const QByteArray& body);

  void handleReply(QNetworkReply* reply);
  void handleHeaderReceived();
  void handleRedirect(const QUrl& url);

 private slots:
  void replyFinished();
  void timeout();
  void sslErrors(const QList<QSslError>& errors);

 signals:
  void requestHeaderReceived(NetworkRequest* request);
  void requestFailed(QNetworkReply::NetworkError error, const QByteArray& data);
  void requestRedirected(NetworkRequest* request, const QUrl& url);
  void requestCompleted(const QByteArray& data);

 private:
  QNetworkRequest m_request;
  QTimer m_timer;

  void enableSSLIntervention();

  QNetworkReply* m_reply = nullptr;
  int m_status = 0;
  bool m_completed = false;
};

#endif  // NETWORKREQUEST_H
