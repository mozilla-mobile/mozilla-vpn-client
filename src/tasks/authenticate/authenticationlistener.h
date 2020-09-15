#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include <QObject>

class QOAuthHttpServerReplyHandler;
class QUrlQuery;

class AuthenticationListener : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationListener(QObject *parent);

    bool initialize();

    void setQueryItems(QUrlQuery &query);

signals:
    void completed(const QString &code);

private:
    QOAuthHttpServerReplyHandler *m_server = nullptr;

    bool m_initialized = false;
};

#endif // AUTHENTICATIONLISTENER_H
