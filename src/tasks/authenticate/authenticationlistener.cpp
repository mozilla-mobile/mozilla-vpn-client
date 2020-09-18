#include "authenticationlistener.h"

#include <limits>
#include <QDebug>
#include <QOAuthHttpServerReplyHandler>
#include <QRandomGenerator>
#include <QUrlQuery>

namespace {

int choosePort(QVector<quint16> triedPorts)
{
    qDebug() << "Choosing port";

    while (true) {
        quint32 v = QRandomGenerator::global()->generate();
        quint16 port = 1024 + (v % (std::numeric_limits<quint16>::max() - 1024));
        qDebug() << "Random port:" << port;

        if (!triedPorts.contains(port)) {
            triedPorts.append(port);
            return port;
        }

        qDebug() << "Already tried!";
    }
}

} // anonymous namespace

AuthenticationListener::AuthenticationListener(QObject *parent) : QObject(parent)
{
    m_server = new QOAuthHttpServerReplyHandler(QHostAddress::LocalHost, this);
    connect(m_server,
            &QAbstractOAuthReplyHandler::callbackReceived,
            [this](const QVariantMap &values) {
                qDebug() << "AuthenticationListener data received:" << values;

                // Unknown connection.
                if (!values.contains("code")) {
                    return;
                }

                QString code = values["code"].toString();
                m_server->close();

                emit completed(code);
            });
}

bool AuthenticationListener::initialize()
{
    qDebug() << "AuthenticationListener initialize";

    Q_ASSERT(!m_initialized);
    m_initialized = true;

    if (m_server->isListening()) {
        return true;
    }

    QVector<quint16> triedPorts;
    for (int i = 0; i < 50; ++i) {
        int port = choosePort(triedPorts);
        if (m_server->listen(QHostAddress::LocalHost, port)) {
            return true;
        }
    }

    qDebug() << "Unable to listen for the authentication server.";
    return false;
}

void AuthenticationListener::setQueryItems(QUrlQuery &query)
{
    Q_ASSERT(m_initialized);
    qDebug() << "Port:" << m_server->port();
    query.addQueryItem("port", QString::number(m_server->port()));
}
