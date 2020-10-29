#ifndef BACKENDLOGSOBSERVER_H
#define BACKENDLOGSOBSERVER_H

#include <functional>
#include <QObject>

class QDBusPendingCallWatcher;

class BackendLogsObserver : public QObject
{
    Q_OBJECT

public:
    BackendLogsObserver(QObject *parent, std::function<void(const QString &)> &&callback);

public slots:
    void completed(QDBusPendingCallWatcher *call);

private:
    std::function<void(const QString &)> m_callback;
};

#endif // BACKENDLOGSOBSERVER_H
