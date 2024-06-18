/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>

#include "socks5.h"

struct Event {
  QString m_newConnection;
  qint64 m_bytesSent;
  qint64 m_bytesReceived;
  qint64 m_when;
};

QList<Event> s_events;

static void cleanup() {
  qint64 now = QDateTime::currentMSecsSinceEpoch();

  QMutableListIterator<Event> i(s_events);
  while (i.hasNext()) {
    if ((now - i.next().m_when) > 1000) {
      i.remove();
    }
  }
}

static QString bytesToString(qint64 bytes) {
  if (bytes < 1024) {
    return QString("%1b").arg(bytes);
  }

  if (bytes < 1024 * 1024) {
    return QString("%1Kb").arg(bytes / 1024);
  }

  if (bytes < 1024 * 1024 * 1024) {
    return QString("%1Mb").arg(bytes / 1024 * 1024);
  }

  return QString("%1Gb").arg(bytes / 1024 * 1024 * 1024);
}

static void update(Socks5* socks5) {
  QString output;

  {
    QTextStream out(&output);
    out << "Connections: " << socks5->connections();

    qint64 bytesSent = 0;
    qint64 bytesReceived = 0;
    QStringList addresses;
    for (const Event& event : s_events) {
      if (!event.m_newConnection.isEmpty() &&
          !addresses.contains(event.m_newConnection)) {
        addresses.append(event.m_newConnection);
      }

      bytesSent += event.m_bytesSent;
      bytesReceived += event.m_bytesReceived;
    }

    out << " [" << addresses.join(", ") << "]";
    out << " Up: " << bytesToString(bytesSent);
    out << " Down: " << bytesToString(bytesReceived);
  }

  output.truncate(80);
  while (output.length() < 80) output.append(' ');
  QTextStream out(stdout);
  out << output << '\r';
}

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);

  Socks5* socks5 = new Socks5(&app, 8888);
  QObject::connect(socks5, &Socks5::connectionsChanged,
                   [&]() { update(socks5); });

  QObject::connect(
      socks5, &Socks5::incomingConnection, [&](const QString& peerAddress) {
        s_events.append(
            Event{peerAddress, 0, 0, QDateTime::currentMSecsSinceEpoch()});
        update(socks5);
      });

  QObject::connect(
      socks5, &Socks5::dataSentReceived, [&](qint64 sent, qint64 received) {
        s_events.append(Event{QString(), sent, received,
                              QDateTime::currentMSecsSinceEpoch()});
        update(socks5);
      });

  QTimer timer;
  QObject::connect(&timer, &QTimer::timeout, [socks5]() {
    cleanup();
    update(socks5);
  });
  timer.start(1000);

  return app.exec();
}
