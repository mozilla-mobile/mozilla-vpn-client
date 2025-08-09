/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "httpconnectionbase.h"

#include <QBuffer>
#include <QUrl>

const QRegularExpression HttpConnectionBase::m_requestRegex =
    QRegularExpression("^([A-Z-]+)\\s([^\\s]+)*\\sHTTP/([0-9].[0-9])");

HttpConnectionBase::HttpConnectionBase(QIODevice* s) : ProxyConnection(s) {
  m_baseUrl.setScheme("http");

  QAbstractSocket* netsock = qobject_cast<QAbstractSocket*>(s);
  if (netsock) {
    m_baseUrl.setHost(netsock->localAddress().toString());
    m_baseUrl.setPort(netsock->localPort());
  } else {
    m_baseUrl.setHost("localhost");
  }
}

void HttpConnectionBase::sendResponse(int code, const QString& message,
                                      const QMap<QString, QString>& headers) {
  QString status = QString("HTTP/1.1 %1 %2\r\n").arg(code).arg(message);
  QByteArray response = status.toUtf8();

  for (auto i = headers.cbegin(); i != headers.cend(); i++) {
    QString header = QString("%1: %2\r\n").arg(i.key()).arg(i.value());
    response.append(header.toUtf8());
  }
  response.append("\r\n");

  m_clientSocket->write(response);
}

void HttpConnectionBase::setError(int code, const QString& message) {
  if (m_state != Proxy) {
    sendResponse(code, message);
  }

  m_errorString = message;
  setState(Closed);
}

void HttpConnectionBase::handshakeRead() {
  while (m_clientSocket->canReadLine()) {
    QString line = m_clientSocket->readLine().trimmed();

    // Read the header line.
    if (m_method.isEmpty()) {
      auto match = m_requestRegex.match(line);
      if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
        setError(400, "Bad Request");
        return;
      }

      m_method = match.captured(1);
      m_uri = match.captured(2);
      m_version = match.captured(3);
      continue;
    }

    // Read request headers.
    if (!line.isEmpty()) {
      qsizetype sep = line.indexOf(':');
      if ((sep > 0) && (sep + 1 < line.length())) {
        QString value = line.sliced(sep + 1).trimmed();
        m_headers.insert(line.first(sep).toLower(), value);
        continue;
      } else {
        setError(400, "Bad Request");
        return;
      }
    }
    QString host = header("Host");
    if (!host.isEmpty()) {
      m_baseUrl.setAuthority(host);
    }

    // Attempt to invoke a meta-method to handle the request.
    // We convert the method into camel-case, prepend 'onHttp'
    // and invoke it as a method taking no arguments.
    bool upper = true;
    QString name = "onHttp";
    for (const QChar c : m_method) {
      if (!c.isLetter()) {
        upper = true;
      } else if (upper) {
        name.append(c.toUpper());
        upper = false;
      } else {
        name.append(c.toLower());
      }
    }
    if (!QMetaObject::invokeMethod(this, qPrintable(name))) {
      setError(405, "Method Not Allowed");
    }
    return;
  }
}

const QString& HttpConnectionBase::header(const QString& name) const {
  auto entry = m_headers.find(name.toLower());
  if (entry != m_headers.end()) {
    return entry.value();
  }
  static const QString empty;
  return empty;
}

void HttpConnectionBase::onHostnameNotFound() { setError(404, "Not Found"); }
