/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationlistener.h"
#include "logger.h"

#ifdef MVPN_ANDROID
#include "platforms/android/androidauthenticationlistener.h"
#endif

#include <limits>
#include <QDesktopServices>
#include <QOAuthHttpServerReplyHandler>
#include <QRandomGenerator>
#include <QUrlQuery>

namespace {

Logger logger(LOG_MAIN, "AuthenticationListener");

int choosePort(QVector<quint16> triedPorts)
{
    logger.log() << "Choosing port";

    while (true) {
        quint32 v = QRandomGenerator::global()->generate();
        quint16 port = 1024 + (v % (std::numeric_limits<quint16>::max() - 1024));
        logger.log() << "Random port:" << port;

        if (!triedPorts.contains(port)) {
            triedPorts.append(port);
            return port;
        }

        logger.log() << "Already tried!";
    }
}

} // anonymous namespace

// static
AuthenticationListener *AuthenticationListener::create(QObject *parent)
{
#ifdef MVPN_ANDROID
    return new AndroidAuthenticationListener(parent);
#else
    return new AuthenticationListener(parent);
#endif
}

AuthenticationListener::AuthenticationListener(QObject *parent) : QObject(parent)
{
    m_server = new QOAuthHttpServerReplyHandler(QHostAddress::LocalHost, this);
    connect(m_server,
            &QAbstractOAuthReplyHandler::callbackReceived,
            [this](const QVariantMap &values) {
                logger.log() << "AuthenticationListener data received";

                // Unknown connection.
                if (!values.contains("code")) {
                    return;
                }

                QString code = values["code"].toString();
                m_server->close();

                emit completed(code);
            });
}

void AuthenticationListener::start(MozillaVPN *vpn, QUrl &url, QUrlQuery &query)
{
    logger.log() << "AuthenticationListener initialize";
    Q_UNUSED(vpn);

    if (!m_server->isListening()) {
        QVector<quint16> triedPorts;
        for (int i = 0; i < 50; ++i) {
            int port = choosePort(triedPorts);
            if (m_server->listen(QHostAddress::LocalHost, port)) {
                break;
            }
        }
    }

    if (!m_server->isListening()) {
        logger.log() << "Unable to listen for the authentication server.";
        emit failed(ErrorHandler::BackendServiceError);
        return;
    }

    logger.log() << "Port:" << m_server->port();
    query.addQueryItem("port", QString::number(m_server->port()));

    url.setQuery(query);
    QDesktopServices::openUrl(url.toString());
}
