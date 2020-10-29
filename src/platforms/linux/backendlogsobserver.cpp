#include "backendlogsobserver.h"
#include "logger.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace {
Logger logger({LOG_LINUX, LOG_CONTROLLER}, "BackendLogsObserver");
}

BackendLogsObserver::BackendLogsObserver(QObject *parent,
                                         std::function<void(const QString &)> &&callback)
    : QObject(parent), m_callback(std::move(callback))
{}

void BackendLogsObserver::completed(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if (reply.isError()) {
        logger.log() << "Error received from the DBus service";
        m_callback("Failed to retrieve logs from the mozillavpn-daemon.");
        return;
    }

    QString status = reply.argumentAt<0>();
    m_callback(status);
}
