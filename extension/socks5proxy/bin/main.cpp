/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QRandomGenerator>
#include <QString>
#include <QTimer>

#include "socks5.h"

#ifdef __linux__
#  include "linuxbypass.h"
#endif
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  include "windowsbypass.h"
#endif

struct Event {
  QString m_newConnection;
  qint64 m_bytesSent;
  qint64 m_bytesReceived;
  qint64 m_when;
};

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

struct CliOptions {
  uint16_t port =
      static_cast<uint16_t>(QRandomGenerator::global()->bounded(49152, 65535));
  QHostAddress addr = QHostAddress::LocalHost;
  QString username = {};
  QString password = {};
  bool verbose = false;
};

static CliOptions parseArgs(const QCoreApplication& app) {
  QCommandLineParser parser;
  parser.setApplicationDescription(
      "A Socks5 Proxy allowing to bypass MozillaVPN");
  parser.addHelpOption();
  // A boolean option with a single name (-p)
  QCommandLineOption portOption({"p", "port"}, "The Port to Listen on", "port");
  parser.addOption(portOption);

  QCommandLineOption addressOption({"a", "address"}, "The Address to Listen on",
                                   "address");
  parser.addOption(addressOption);

  QCommandLineOption userOption({"u", "user"}, "The Username", "username");
  parser.addOption(userOption);

  QCommandLineOption passOption({"P", "password"}, "The password", "password");
  parser.addOption(passOption);

  QCommandLineOption verboseOption({"v", "verbose"}, "Verbose");
  parser.addOption(verboseOption);
  parser.process(app);

  CliOptions out = {};
  if (parser.isSet(portOption)) {
    auto portString = parser.value(portOption);
    const auto p = portString.toInt();
    if (p > 65535 || p <= 0) {
      qFatal("Port is Not Valid");
    }
    out.port = p;
  }
  if (parser.isSet(addressOption)) {
    auto valueString = parser.value(addressOption);
    out.addr = QHostAddress(valueString);
  }
  if (parser.isSet(userOption)) {
    out.username = parser.value(userOption);
  }
  if (parser.isSet(passOption)) {
    out.password = parser.value(passOption);
  }
  if (parser.isSet(verboseOption)) {
    out.verbose = true;
  }
  return out;
};

static void startVerboseCLI(const Socks5* socks5) {
  static QList<Event> s_events;
  static QTimer timer;

  auto cleanup = []() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QMutableListIterator<Event> i(s_events);
    while (i.hasNext()) {
      if ((now - i.next().m_when) > 1000) {
        i.remove();
      }
    }
  };

  auto printStatus = [socks5]() {
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
  };
  QObject::connect(socks5, &Socks5::connectionsChanged,
                   [printStatus]() { printStatus(); });
  QObject::connect(
      socks5, &Socks5::incomingConnection,
      [printStatus](QAbstractSocket* s, const QHostAddress& peer) {
        Q_UNUSED(s);
        s_events.append(
            Event{peer.toString(), 0, 0, QDateTime::currentMSecsSinceEpoch()});
        printStatus();
      });

  QObject::connect(
      socks5, &Socks5::dataSentReceived,
      [printStatus](qint64 sent, qint64 received) {
        s_events.append(Event{QString(), sent, received,
                              QDateTime::currentMSecsSinceEpoch()});
        printStatus();
      });

  QObject::connect(&timer, &QTimer::timeout, [printStatus, cleanup]() {
    cleanup();
    printStatus();
  });
  timer.start(1000);
}

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("socksproxy");
  QCoreApplication::setApplicationVersion("0.1");
  auto const config = parseArgs(app);

  if (!config.username.isEmpty() || !config.password.isEmpty()) {
    // Todo: actually do auth.
    qFatal("AAH NOT IMPLENTED SORRYY");
    return 1;
  }

  Socks5* socks5 = new Socks5(config.port, config.addr, &app);
  if (config.verbose) {
    startVerboseCLI(socks5);
  }
  qDebug() << "Starting on port" << QString::number(config.port);

  QObject::connect(socks5, &Socks5::incomingConnection,
                   [](QAbstractSocket* s, const QHostAddress& peer) {
                     Q_UNUSED(s);
                     qDebug() << "Connection from  on port" << peer.toString();
                   });

#ifdef __linux__
  LinuxBypass* bypass = new LinuxBypass(socks5);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  WindowsBypass* bypass = new WindowsBypass(socks5);
#endif

  return app.exec();
}
