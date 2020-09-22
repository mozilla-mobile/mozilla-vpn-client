#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include "errorhandler.h"

#include <QObject>

class QOAuthHttpServerReplyHandler;
class QUrl;
class QUrlQuery;

class AuthenticationListener : public QObject
{
    Q_OBJECT

public:
    explicit AuthenticationListener(QObject *parent) : QObject(parent) {}

    void start(QUrl &url, QUrlQuery &query);

signals:
    void completed(const QString &code);

    void failed(ErrorHandler::ErrorType error);

private:
    QOAuthHttpServerReplyHandler *m_server = nullptr;
};

#endif // AUTHENTICATIONLISTENER_H
