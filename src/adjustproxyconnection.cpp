/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is not afully functional http server

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

const QString HTTP_RESPONSE =
    "HTTP/1.1 %1\nContent-Type: "
    "application/json\n\n%2\n";

const QList<QString> allowList = {"adid",
                                  "app_token",
                                  "attribution_deeplink",
                                  "bundle_id",
                                  "device_type",
                                  "environment",
                                  "event_token",
                                  "idfv",
                                  "needs_response_details",
                                  "os_name",
                                  "os_version",
                                  "package_name",
                                  "reference_tag",
                                  "tracking_enabled",
                                  "zone_offset",
                                  "att_status"};

const QList<QPair<QString, QString>> defaultValues = {
    QPair<QString, QString>("app_name", "default"),
    QPair<QString, QString>("app_version", "2.0"),
    QPair<QString, QString>("app_version_short", "2.0"),
    QPair<QString, QString>("base_amount", "0"),
    QPair<QString, QString>("device_name", "default"),
    QPair<QString, QString>("engagement_type", "0"),
    QPair<QString, QString>("event_buffering_enabled", "0"),
    QPair<QString, QString>("event_cost_id", "xxxxx"),
    QPair<QString, QString>("ios_uuid", "xxxxx"),
    QPair<QString, QString>("manufacturer", "default"),
    QPair<QString, QString>("nonce", "0"),
    QPair<QString, QString>("platform", "Default"),
    QPair<QString, QString>("random_user_id", "xxxxx"),
    QPair<QString, QString>("region", "xxxxx"),
    QPair<QString, QString>("store_name", "xxxxx"),
    QPair<QString, QString>("terms_signed", "0"),
    QPair<QString, QString>("time_spent", "0"),
    QPair<QString, QString>("tracker_token", "xxxxx")};

QHash<QString, QString> denyList;

namespace {
Logger logger(LOG_ADJUST, "AdjustProxyConnection");
}  // namespace

AdjustProxyConnection::AdjustProxyConnection(QObject* parent,
                                             QTcpSocket* connection)
    : QObject(parent), m_connection(connection) {
  MVPN_COUNT_CTOR(AdjustProxyConnection);

  logger.debug() << "New connection received";

  for (const auto entry : defaultValues) {
    denyList.insert(entry.first, entry.second);
  }

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
      m_contentLength = headerPair.second.toInt(&ok, 10);
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
  if (m_buffer.trimmed().length() > m_contentLength) {
    logger.error() << "Buffer longer than the declared Contend-Length";
    m_connection->close();
    return;
  }

  if (m_method == "GET") {
    m_parameters = QUrlQuery(m_route);
  } else {
    if (m_buffer.trimmed().length() < m_contentLength) {
      return;
    }
    m_parameters = QUrlQuery(m_buffer.trimmed());
  }

  m_state = ProcessingState::ParametersDone;
}

void AdjustProxyConnection::filterParametersAndForwardRequest() {
  QList<QPair<QString, QString>> newParameters;
  QList<QPair<QString, QString>> unknownParameters;

  for (QPair<QString, QString> parameter : m_parameters.queryItems()) {
    if (allowList.contains(parameter.first)) {
      newParameters.append(
          QPair<QString, QString>(parameter.first, parameter.second));
    } else if (denyList.contains(parameter.first)) {
      newParameters.append(
          QPair<QString, QString>(parameter.first, denyList[parameter.first]));
    } else {
      unknownParameters.append(
          QPair<QString, QString>(parameter.first, parameter.second));
    }
  }

  m_parameters.setQueryItems(newParameters);
  m_unknownParameters.setQueryItems(unknownParameters);

  forwardRequest();
}

void AdjustProxyConnection::forwardRequest() {
  NetworkRequest* request;

  request = NetworkRequest::createForAdjustProxy(
      this, m_method, m_route.toString(), m_headers, m_parameters.toString(),
      m_unknownParameters.toString());

  connect(
      request, &NetworkRequest::requestFailed,
      [this, request](QNetworkReply::NetworkError, const QByteArray& data) {
        logger.debug() << "Adjust Proxy request completed with: "
                       << request->statusCode() << ", "
                       << data.replace('\n', ' ');
        m_connection->write(
            HTTP_RESPONSE.arg(QByteArray::number(request->statusCode()), data)
                .toUtf8());
        m_connection->close();
      });

  connect(
      request, &NetworkRequest::requestCompleted,
      [this, request](const QByteArray& data) {
        logger.debug() << "Adjust Proxy request completed with: "
                       << request->statusCode() << ", "
                       << data.replace('\n', ' ');
        m_connection->write(
            HTTP_RESPONSE.arg(QByteArray::number(request->statusCode()), data)
                .toUtf8());
        m_connection->close();
      });
}
