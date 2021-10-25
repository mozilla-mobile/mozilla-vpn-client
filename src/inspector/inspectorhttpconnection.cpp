/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhttpconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QFile>
#include <QHostAddress>
#include <QTcpSocket>

constexpr const char* HTTP_404_RESPONSE =
    "HTTP/1.1 404 Not Found\nServer: mozillavpn\nContent-Type: "
    "text/html\n\n<html><h1>Not found</h1></html>\n";

namespace {
Logger logger(LOG_INSPECTOR, "InspectorHttpConnection");

struct Path {
  QString m_path;
  QString m_file;
  QString m_mime;
};

static QList<Path> s_paths{
    Path{"/", ":webserver/index.html", "text/html; charset=UTF-8"},
    Path{"/index.html", ":webserver/index.html", "text/html"},
};

}  // namespace

InspectorHttpConnection::InspectorHttpConnection(QObject* parent,
                                                 QTcpSocket* connection)
    : QObject(parent), m_connection(connection) {
  MVPN_COUNT_CTOR(InspectorHttpConnection);

#if !defined(MVPN_ANDROID) && !defined(MVPN_IOS)
  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  Q_ASSERT(connection->localAddress() == QHostAddress("::ffff:127.0.0.1") ||
           connection->localAddress() == QHostAddress::LocalHost ||
           connection->localAddress() == QHostAddress::LocalHostIPv6);
#endif

  logger.debug() << "New connection received";

  Q_ASSERT(m_connection);
  connect(m_connection, &QTcpSocket::readyRead, this,
          &InspectorHttpConnection::readData);
}

InspectorHttpConnection::~InspectorHttpConnection() {
  MVPN_COUNT_DTOR(InspectorHttpConnection);
  logger.debug() << "Connection released";
}

void InspectorHttpConnection::readData() {
  Q_ASSERT(m_connection);
  QByteArray input = m_connection->readAll();
  m_buffer.append(input);

  while (true) {
    int pos = m_buffer.indexOf("\n");
    if (pos == -1) {
      break;
    }

    QByteArray line = m_buffer.left(pos);
    m_buffer.remove(0, pos + 1);

    QString header = QString(line).trimmed();
    if (header == "") {
      processHeaders();
      break;
    }

    m_headers.append(header);
  }
}

void InspectorHttpConnection::processHeaders() {
  logger.debug() << "process headers:" << m_headers;

  if (m_headers.isEmpty()) {
    m_connection->close();
    return;
  }

  QStringList parts = m_headers[0].split(" ");
  if (parts[0] != "GET" || parts.length() < 2) {
    m_connection->write(HTTP_404_RESPONSE);
    m_connection->close();
    return;
  }

  QString path = parts[1];
  for (const Path& p : s_paths) {
    if (path == p.m_path) {
      QFile file(p.m_file);

      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logger.error() << "Unable to read file" << p.m_file;
        break;
      }

      QByteArray content = file.readAll();

      QByteArray response;
      {
        QTextStream out(&response);
        out << "HTTP/1.1 200 OK\n";
        out << "Content-Type: " << p.m_mime << "\n";
        out << "Server: mozillavpn\n\n";
        out << content;
      }

      m_connection->write(response);
      m_connection->close();
      return;
    }
  }

  m_connection->write(HTTP_404_RESPONSE);
  m_connection->close();
}
