/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "desktopauthenticationlistener.h"

#include <QOAuthHttpServerReplyHandler>
#include <QRandomGenerator>
#include <QUrlQuery>
#include <limits>

#include "leakdetector.h"
#include "logging/logger.h"
#include "urlopener.h"

namespace {
Logger logger("DesktopAuthenticationListener");
}

DesktopAuthenticationListener::DesktopAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MZ_COUNT_CTOR(DesktopAuthenticationListener);

  m_server = new QOAuthHttpServerReplyHandler(QHostAddress::LocalHost, this);
  connect(m_server, &QAbstractOAuthReplyHandler::callbackReceived, this,
          [this](const QVariantMap& values) {
            logger.debug() << "DesktopAuthenticationListener data received";

            // Unknown connection.
            if (!values.contains("code")) {
              return;
            }

            QString code = values["code"].toString();

            emit completed(code);
          });
}

DesktopAuthenticationListener::~DesktopAuthenticationListener() {
  MZ_COUNT_DTOR(DesktopAuthenticationListener);
}

void DesktopAuthenticationListener::start(Task* task,
                                          const QString& codeChallenge,
                                          const QString& codeChallengeMethod,
                                          const QString& emailAddress) {
  logger.debug() << "DesktopAuthenticationListener initialize";

  Q_UNUSED(task);

  QUrl url(createAuthenticationUrl(codeChallenge, codeChallengeMethod,
                                   emailAddress));

  if (!m_server->isListening()) {
    m_server->listen(QHostAddress::LocalHost);
  }

  if (!m_server->isListening()) {
    logger.error() << "Unable to listen for the authentication server.";
    emit failed(ErrorHandler::UnrecoverableError);
    return;
  }

  logger.debug() << "Port:" << m_server->port();

  QUrlQuery query(url.query());
  query.addQueryItem("port", QString::number(m_server->port()));
  query.addQueryItem("utm_medium", "vpn-client");
  query.addQueryItem("utm_source", "desktop-signup-flow");
  url.setQuery(query);

  UrlOpener::instance()->openUrl(url);
}
