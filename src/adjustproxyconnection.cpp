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
#include <QUrl>
#include <QUrlQuery>

constexpr const char* HTTP_RESPONSE =
    "HTTP/1.1 <status>\nContent-Type: "
    "application/json\n\n<message>\n";

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
  Q_ASSERT(m_connection);
  QByteArray input = m_connection->readAll();
  m_buffer.append(input);

  switch (m_state) {
    case ProcessingState::NotStarted:
      if (m_buffer.isEmpty()) {
        return;
      }
      processFirstLine();
    case ProcessingState::FirstLineDone:
      if (m_buffer.isEmpty()) {
        return;
      }
      processHeaders();
    case ProcessingState::HeadersDone:
      processParameters();
    case ProcessingState::ParametersDone:
      break;
    default:
      logger.debug() << "Unknown ProcessingState: " << m_state;
  }

  if (m_state == ProcessingState::ParametersDone) {
    filterParameters();
  }
}

void AdjustProxyConnection::processFirstLine() {
  int pos = m_buffer.indexOf("\n");
  if (pos == -1) {
    return;
  }

  QByteArray line = m_buffer.left(pos);
  m_buffer.remove(0, pos + 1);

  QList<QByteArray> parts = line.split(' ');
  if (parts.length() < 2) {
    m_connection->close();
    return;
  }

  m_method.append(parts[0]);
  m_route = parts[1];

  m_state = ProcessingState::FirstLineDone;
}

void AdjustProxyConnection::processHeaders() {
  while (true) {
    int pos = m_buffer.indexOf("\n");
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

    QPair<QByteArray, QByteArray> headerPair;
    headerPair.first = headerName;
    headerPair.second = headerValue;

    m_headers.append(headerPair);
  }

  m_state = ProcessingState::HeadersDone;
}

void AdjustProxyConnection::processParameters() {
  if (m_method == "GET") {
    m_paramters = QUrlQuery(m_route);
  } else {
    if (m_buffer.isEmpty()) {
      return;
    }
    m_paramters = QUrlQuery(m_buffer.trimmed());
  }

  m_state = ProcessingState::ParametersDone;
}

void AdjustProxyConnection::filterParameters() {
  QList<QPair<QString, QString>> newParameters;

  for (QPair<QString, QString> parameter : m_paramters.queryItems()) {
    if (parameter.first == "adid" || parameter.first == "app_token" ||
        parameter.first == "attribution_deeplink" ||
        parameter.first == "bundle_id" || parameter.first == "device_type" ||
        parameter.first == "environment" || parameter.first == "event_token" ||
        parameter.first == "idfv" ||
        parameter.first == "needs_response_details" ||
        parameter.first == "os_name" || parameter.first == "os_version" ||
        parameter.first == "package name" ||
        parameter.first == "reference_tag" ||
        parameter.first == "tracking_enabled" ||
        parameter.first == "zone_offset" || parameter.first == "att_status") {
      newParameters.append(QPair(parameter.first, parameter.second));
    } else {
      newParameters.append(
          QPair(parameter.first, QByteArray(parameter.second.size(), '0')));
    }
  }

  m_paramters.setQueryItems(newParameters);

  forwardRequest();
}

void AdjustProxyConnection::forwardRequest() {
  NetworkRequest* request;
  if (m_method == "GET") {
    m_route.setQuery(m_paramters);

    request =
        NetworkRequest::createForAdjustForwardGet(this, m_route, m_headers);
  } else if (m_method == "POST") {
    request = NetworkRequest::createForAdjustForwardPost(
        this, m_route, m_headers, m_paramters.toString().toUtf8());
  } else {
    logger.warning() << "Method not supported!";
    return;
  }

  connect(
      request, &NetworkRequest::requestFailed,
      [this](QNetworkReply::NetworkError, const QByteArray& data, int status) {
        QByteArray response(HTTP_RESPONSE);
        response.replace("<status>", QByteArray::number(status));
        response.replace("<message>", data);
        m_connection->write(response);
        m_connection->close();
      });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.debug() << "Adjust request succeeded";
            logger.debug() << data;
            QByteArray response(HTTP_RESPONSE);
            response.replace("<status>", "200");
            response.replace("<message>", data);
            m_connection->write(response);
            m_connection->close();
          });
}
