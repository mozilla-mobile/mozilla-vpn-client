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
  logger.debug() << "New data read";
  Q_ASSERT(m_connection);
  QByteArray input = m_connection->readAll();
  m_buffer.append(input);

  switch (m_state) {
    case ProcessingState::NotStarted:
      processFirstLine();
      [[fallthrough]];
    case ProcessingState::FirstLineDone:
      processHeaders();
      [[fallthrough]];
    case ProcessingState::HeadersDone:
      processParameters();
      [[fallthrough]];
    case ProcessingState::ParametersDone:
      filterParametersAndForwardRequest();
      break;
    default:
      Q_ASSERT(false);
      logger.debug() << "Unknown ProcessingState: " << m_state;
  }
}

void AdjustProxyConnection::processFirstLine() {
  logger.debug() << "Processing first line";

  if (m_buffer.isEmpty()) {
    return;
  }

  int pos = m_buffer.indexOf("\n");
  if (pos == -1) {
    return;
  }

  QByteArray line = m_buffer.left(pos);
  m_buffer.remove(0, pos + 1);

  QList<QByteArray> parts = line.split(' ');
  if (parts.length() < 2) {
    logger.error() << "Invalid HTTP request";
    m_connection->close();
    return;
  }

  m_method.append(parts[0]);
  m_route = parts[1].trimmed();

  m_state = ProcessingState::FirstLineDone;
}

void AdjustProxyConnection::processHeaders() {
  logger.debug() << "Processing headers";

  if (m_buffer.isEmpty()) {
    return;
  }

  while (true) {
    int pos = m_buffer.indexOf("\n");
    if (pos == -1) {
      return;
    }

    QByteArray line = m_buffer.left(pos);
    m_buffer.remove(0, pos + 1);

    QByteArray header = line.trimmed();
    if (header.isEmpty()) {
      break;
    }

    pos = header.indexOf(":");
    if (pos == -1) {
      continue;
    }

    QByteArray headerName = header.left(pos);
    header.remove(0, pos + 1);
    QByteArray headerValue = header.trimmed();

    QPair<QString, QString> headerPair;
    headerPair.first = QString(headerName);
    headerPair.second = QString(headerValue);

    if (QString::compare(headerPair.first, "host", Qt::CaseInsensitive) == 0) {
      continue;
    }

    if (QString::compare(headerPair.first, "content-length",
                         Qt::CaseInsensitive) == 0) {
      bool ok;
      m_contentLength = headerPair.second.toUInt(&ok, 10);
      if (!ok) {
        logger.error() << "Content Length could not be parsed";
        m_connection->close();
        return;
      }
      continue;
    }

    m_headers.append(headerPair);
  }

  m_state = ProcessingState::HeadersDone;
}

void AdjustProxyConnection::processParameters() {
  logger.debug() << "Processing parameters";

  uint32_t bodyLength = m_buffer.trimmed().length();

  if (bodyLength > m_contentLength) {
    logger.error() << "Buffer longer than the declared Contend-Length";
    m_connection->close();
    return;
  }

  if (bodyLength < m_contentLength) {
    return;
  }
  m_bodyParameters = QUrlQuery(m_buffer.trimmed());

  m_queryParameters = QUrlQuery(m_route);

  m_state = ProcessingState::ParametersDone;
}

void AdjustProxyConnection::filterParametersAndForwardRequest() {
  logger.debug() << "Filtering parameters";

  m_queryParameters =
      AdjustFiltering::filterParameters(m_queryParameters, m_unknownParameters);
  m_bodyParameters =
      AdjustFiltering::filterParameters(m_bodyParameters, m_unknownParameters);

  forwardRequest();
}

void AdjustProxyConnection::forwardRequest() {
  logger.debug() << "Forwarding request";

  NetworkRequest* request;

  QString headersString;

  for (QPair<QString, QString> header : m_headers) {
    headersString.append(header.first);
    headersString.append(": ");
    headersString.append(header.second);
    headersString.append(", ");
  }

  logger.debug() << "Sending Adjust request with: " << m_method << ", "
                 << headersString
                 << logger.sensitive(m_queryParameters.toString()) << ", "
                 << logger.sensitive(m_bodyParameters.toString());

  request = NetworkRequest::createForAdjustProxy(
      this, m_method, m_route.toString(), m_headers,
      m_queryParameters.toString(), m_bodyParameters.toString(),
      m_unknownParameters);

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
