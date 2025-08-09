/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "httprequest.h"

#include <QIODevice>
#include <QRegularExpression>

const QRegularExpression HttpRequest::m_regex =
    QRegularExpression("^([A-Z-]+)\\s([^\\s]+)*\\sHTTP/([0-9].[0-9])");

HttpRequest::HttpRequest(const QString& line) {
  auto match = m_regex.match(line);
  if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
    return;
  }

  m_method = match.captured(1);
  m_uri = match.captured(2);
  m_verMajor = match.captured(3)[0].digitValue();
  m_verMinor = match.captured(3)[2].digitValue();
}

bool HttpRequest::isValid() const {
  return !m_method.isEmpty() && !m_uri.isEmpty() && (m_verMajor > 0) &&
         (m_verMinor >= 0);
}

// Peek at the socket and determine if this is a socks connection.
HttpRequest HttpRequest::peek(QIODevice* socket) {
  if (!socket->canReadLine()) {
    return HttpRequest();
  }

  // Read one line and then roll it back.
  socket->startTransaction();
  auto guard = qScopeGuard([socket]() { socket->rollbackTransaction(); });
  return HttpRequest(QString(socket->readLine()));
}

bool HttpRequest::addHeader(const QString& line) {
  qsizetype sep = line.indexOf(':');
  if ((sep <= 0) || ((sep + 1) >= line.length())) {
    return false;
  }
  QString value = line.sliced(sep + 1).trimmed();
  m_headers.insert(line.first(sep).toLower(), value);
  return true;
}

const QString& HttpRequest::header(const QString& name) const {
  auto entry = m_headers.find(name.toLower());
  if (entry != m_headers.end()) {
    return entry.value();
  }
  static const QString empty;
  return empty;
}
