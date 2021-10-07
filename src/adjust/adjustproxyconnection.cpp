/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is not a fully functional http server
// It is just a proxy server designed to work with the Adjust SDK

#include "adjustproxyconnection.h"
#include "adjustfiltering.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "networkrequest.h"

#include <QFile>
#include <QHostAddress>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>

const QString HTTP_RESPONSE(
    "HTTP/1.1 %1\nContent-Type: application/json\n\n%2\n");

namespace {
Logger logger(LOG_ADJUST, "AdjustProxyConnection");
}  // namespace

AdjustProxyConnection::AdjustProxyConnection(QObject* parent,
                                             QTcpSocket* connection)
    : QObject(parent), m_connection(connection), m_packageHandler(this) {
  MVPN_COUNT_CTOR(AdjustProxyConnection);

  logger.debug() << "New connection received";

  Q_ASSERT(m_connection);
  connect(m_connection, &QTcpSocket::readyRead, this,
          &AdjustProxyConnection::readData);
}

AdjustProxyConnection::~AdjustProxyConnection() {
  MVPN_COUNT_DTOR(AdjustProxyConnection);
  logger.debug() << "Connection released";
}

void AdjustProxyConnection::readData() {
  logger.debug() << "New data read";
  Q_ASSERT(m_connection);
  QByteArray input = m_connection->readAll();

  m_packageHandler.processData(input);

  if (m_packageHandler.isInvalidRequest()) {
    m_connection->close();
    return;
  }

  if (m_packageHandler.isProcessingDone()) {
    forwardRequest();
  }
}

void AdjustProxyConnection::forwardRequest() {
  logger.debug() << "Forwarding request";

  QString headersString;
  QList<QPair<QString, QString>> headers = m_packageHandler.getHeaders();

  for (QPair<QString, QString> header : headers) {
    headersString.append(header.first);
    headersString.append(": ");
    headersString.append(header.second);
    headersString.append(", ");
  }

  QString method = m_packageHandler.getMethod();
  QString route = m_packageHandler.getRoute();
  QString queryParameters = m_packageHandler.getQueryParameters();
  QString bodyParameters = m_packageHandler.getBodyParameters();
  QStringList unknownParameters = m_packageHandler.getUnknownParameters();

  logger.debug() << "Sending Adjust request with: " << method << ", " << route
                 << ", " << headersString << logger.sensitive(queryParameters)
                 << ", " << logger.sensitive(bodyParameters) << ", "
                 << unknownParameters;

  NetworkRequest* request = NetworkRequest::createForAdjustProxy(
      this, method, route, headers, queryParameters, bodyParameters,
      unknownParameters);

  connect(
      request, &NetworkRequest::requestFailed,
      [this, request](QNetworkReply::NetworkError, const QByteArray& data) {
        logger.debug() << "Adjust Proxy request completed with: "
                       << request->statusCode() << ", " << data;
        m_connection->write(
            HTTP_RESPONSE.arg(QByteArray::number(request->statusCode()), data)
                .toUtf8());
        m_connection->close();
      });

  connect(
      request, &NetworkRequest::requestCompleted,
      [this, request](const QByteArray& data) {
        logger.debug() << "Adjust Proxy request completed with: "
                       << request->statusCode() << ", " << data;
        m_connection->write(
            HTTP_RESPONSE.arg(QByteArray::number(request->statusCode()), data)
                .toUtf8());
        m_connection->close();
      });
}
