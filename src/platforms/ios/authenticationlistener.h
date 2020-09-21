#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include <QObject>

class QOAuthHttpServerReplyHandler;
class QUrl;
class QUrlQuery;

class AuthenticationListener : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationListener(QObject *parent);

    bool start(QUrl &url, QUrlQuery &query);

signals:
    void completed(const QString &code);

private:
    QOAuthHttpServerReplyHandler *m_server = nullptr;
};

#endif // AUTHENTICATIONLISTENER_H
