#ifndef NETWORKREQUEST_H
#define NETWORKREQUEST_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

class MozillaVPN;
class QNetworkAccessManager;

class NetworkRequest : public QObject
{
    Q_OBJECT

public:
    ~NetworkRequest() = default;

    // This object deletes itself at the end of the operation.
    static NetworkRequest *createForAuthenticate(MozillaVPN *vpn);

    static NetworkRequest *createForAuthenticationVerification(QObject *parent,
                                                               const QString &verificationUrl);

    static NetworkRequest *createForDeviceCreation(MozillaVPN *vpn,
                                                   const QString &deviceName,
                                                   const QString &pubKey);

private:
    NetworkRequest(QObject *parent);

private Q_SLOTS:
    void replyFinished(QNetworkReply *reply);

signals:
    void requestFailed(QNetworkReply::NetworkError error);
    void requestCompleted(QByteArray data);

private:
    QNetworkAccessManager *m_manager;
    QNetworkRequest m_request;
};

#endif // NETWORKREQUEST_H
