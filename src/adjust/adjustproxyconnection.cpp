/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is not a fully functional http server
// It is just a proxy server designed to work with the Adjust SDK

#include "adjustproxyconnection.h"
#include "adjustfiltering.h"
#include "adjusttasksubmission.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "taskscheduler.h"

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
  const QList<QPair<QString, QString>>& headers = m_packageHandler.getHeaders();

  for (QPair<QString, QString> header : headers) {
    headersString.append(header.first);
    headersString.append(": ");
    headersString.append(header.second);
    headersString.append(", ");
  }

  const QString& method = m_packageHandler.getMethod();
  const QString& path = m_packageHandler.getPath();
  const QString queryParameters = m_packageHandler.getQueryParameters();
  const QString bodyParameters = m_packageHandler.getBodyParameters();
  const QStringList& unknownParameters =
      m_packageHandler.getUnknownParameters();

  logger.debug() << "Sending Adjust request with: " << method << ", " << path
                 << ", " << headersString << logger.sensitive(queryParameters)
                 << ", " << logger.sensitive(bodyParameters) << ", "
                 << unknownParameters;

  AdjustTaskSubmission* task =
      new AdjustTaskSubmission(method, path, headers, queryParameters,
                               bodyParameters, unknownParameters);

  connect(
      task, &AdjustTaskSubmission::operationCompleted, this,
      [this](const QByteArray& data, int statusCode) {
        m_connection->write(
            HTTP_RESPONSE.arg(QByteArray::number(statusCode), data).toUtf8());
        m_connection->close();
      });

  TaskScheduler::scheduleTask(task);
}
