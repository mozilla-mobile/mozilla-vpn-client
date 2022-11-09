/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjustproxypackagehandler.h"
#include "adjustfiltering.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"

#include <QUrl>
#include <QUrlQuery>

namespace {
Logger logger(LOG_ADJUST, "AdjustProxyPackageHandler");
}  // namespace

AdjustProxyPackageHandler::AdjustProxyPackageHandler() {
  MVPN_COUNT_CTOR(AdjustProxyPackageHandler);

  logger.debug() << "New package handler created";
}

AdjustProxyPackageHandler::~AdjustProxyPackageHandler() {
  MVPN_COUNT_DTOR(AdjustProxyPackageHandler);
  logger.debug() << "Package handler destroyed";
}

void AdjustProxyPackageHandler::processData(const QByteArray& input) {
  logger.debug() << "Processing new data";
  m_buffer.append(input);

  switch (m_state) {
    case ProcessingState::NotStarted:
      if (!processFirstLine()) break;
      [[fallthrough]];
    case ProcessingState::FirstLineDone:
      if (!processHeaders()) break;
      [[fallthrough]];
    case ProcessingState::HeadersDone:
      if (!processParameters()) break;
      [[fallthrough]];
    case ProcessingState::ParametersDone:
      filterParameters();
      [[fallthrough]];
    case ProcessingState::ProcessingDone:
      logger.debug() << "Processing is done";
      break;
    case ProcessingState::InvalidRequest:
      logger.debug() << "Invalid request state; connection should be closed";
      break;
    default:
      Q_ASSERT(false);
      logger.debug() << "Unknown ProcessingState: " << m_state;
  }
}

bool AdjustProxyPackageHandler::processFirstLine() {
  logger.debug() << "Processing first line";

  if (m_buffer.isEmpty()) {
    return false;
  }

  qsizetype pos = m_buffer.indexOf("\n");
  if (pos == -1) {
    return false;
  }

  QByteArray line = m_buffer.left(pos);
  m_buffer.remove(0, pos + 1);

  QList<QByteArray> parts = line.split(' ');
  if (parts.length() < 2) {
    logger.error() << "Invalid HTTP request; connection should be closed";
    m_state = ProcessingState::InvalidRequest;
    return false;
  }

  m_method.append(parts[0]);
  m_route = parts[1].trimmed();
  m_path = m_route.path();

  m_state = ProcessingState::FirstLineDone;
  logger.debug() << m_method << ", " << m_path;
  return true;
}

bool AdjustProxyPackageHandler::processHeaders() {
  logger.debug() << "Processing headers";

  if (m_buffer.isEmpty()) {
    return false;
  }

  while (true) {
    qsizetype pos = m_buffer.indexOf("\n");
    if (pos == -1) {
      return false;
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
        logger.error() << "Content Length could not be parsed; connection "
                          "should be closed";
        m_state = ProcessingState::InvalidRequest;
        return false;
      }
      continue;
    }

    m_headers.append(headerPair);
  }

  m_state = ProcessingState::HeadersDone;
  return true;
}

bool AdjustProxyPackageHandler::processParameters() {
  logger.debug() << "Processing parameters";

  qsizetype bodyLength = m_buffer.trimmed().length();

  if (bodyLength > m_contentLength) {
    logger.error() << "Buffer longer than the declared Contend-Length; "
                      "connection should be closed";
    m_state = ProcessingState::InvalidRequest;
    return false;
  }

  if (bodyLength < m_contentLength) {
    return false;
  }
  m_bodyParameters = QUrlQuery(m_buffer.trimmed());

  m_queryParameters = QUrlQuery(m_route);

  m_state = ProcessingState::ParametersDone;
  return true;
}

void AdjustProxyPackageHandler::filterParameters() {
  logger.debug() << "Filtering parameters";

  m_queryParameters = AdjustFiltering::instance()->filterParameters(
      m_queryParameters, m_unknownParameters);
  m_bodyParameters = AdjustFiltering::instance()->filterParameters(
      m_bodyParameters, m_unknownParameters);

  m_state = ProcessingState::ProcessingDone;
}
