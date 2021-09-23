/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjustproxyconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "networkrequest.h"

#include <QFile>
#include <QHostAddress>
#include <QTcpSocket>

constexpr const char* HTTP_200_RESPONSE =
    "HTTP/1.1 200 OK\nContent-Type: "
    "application/json\n\n{}\n";

namespace {
Logger logger(LOG_MAIN, "AdjustProxyConnection");
}  // namespace

AdjustProxyConnection::AdjustProxyConnection(QObject* parent,
                                             QTcpSocket* connection)
    : QObject(parent), m_connection(connection) {
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
  Q_ASSERT(m_connection);
  QByteArray input = m_connection->readAll();
  m_buffer.append(input);

  // Parsing method and route
  int pos = m_buffer.indexOf("\n");
  if (pos == -1) {
    return;
  }

  QByteArray line = m_buffer.left(pos);
  m_buffer.remove(0, pos + 1);

  QList<QByteArray> parts = line.split(' ');
  if (parts.length() < 2) {
    return;
  }

  m_method.append(parts[0]);
  m_route.append(parts[1]);

  if (m_method == "GET") {
    pos = m_route.indexOf('?');
    if (pos > -1) {
      m_parametersString = m_route.right(m_route.length() - (pos + 1));
      m_route.remove(pos, m_route.length());
    }
  }

  // Parsing headers
  while (true) {
    pos = m_buffer.indexOf("\n");
    if (pos == -1) {
      break;
    }

    QByteArray line = m_buffer.left(pos);
    m_buffer.remove(0, pos + 1);

    QByteArray header = line.trimmed();
    if (header == "") {
      break;
    }

    pos = header.indexOf(":");
    if (pos == -1) {
      continue;
    }

    QByteArray headerName = header.left(pos);
    header.remove(0, pos + 1);
    QByteArray headerValue = header.trimmed();

    m_headers.insert(headerName, headerValue);
  }

  if (m_parametersString.isEmpty()) {
    m_parametersString = m_buffer.trimmed();
  }

  // Parsing parameters
  while (true) {
    pos = m_parametersString.indexOf("&");
    if (pos == -1) {
      break;
    }

    QByteArray line = m_parametersString.left(pos);
    m_parametersString.remove(0, pos + 1);

    QByteArray property = line.trimmed();
    if (property == "") {
      break;
    }

    pos = property.indexOf("=");
    if (pos == -1) {
      continue;
    }

    QByteArray propertyName = property.left(pos);
    property.remove(0, pos + 1);
    QByteArray propertyValue = property.trimmed();

    m_paramters.insert(propertyName, propertyValue);
  }

  filterParameters();
}

void AdjustProxyConnection::filterParameters() { forwardRequest(); }

void AdjustProxyConnection::forwardRequest() {
  NetworkRequest* request;
  if (m_method == "GET") {
    request = NetworkRequest::createForAdjustForwardGet(
        this, m_route.append(m_parametersString), m_headers);
  } else if (m_method == "POST") {
    request = NetworkRequest::createForAdjustForwardPost(
        this, m_route, m_headers, m_parametersString);
  } else {
    logger.warning() << "Method not supported!";
    return;
  }

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed adjust request" << error;
            logger.debug() << data;
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.debug() << "Adjust request succeeded";
            logger.debug() << data;
            m_connection->write(HTTP_200_RESPONSE);
            m_connection->close();
          });
}
