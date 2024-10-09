/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QLocalServer>
#include <QRandomGenerator>
#include <QString>
#include <QTcpServer>
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
  qint64 m_when;
};

class BoxcarAverage final {
  public:
   BoxcarAverage(int buckets = 8) : m_buckets(buckets) { advance(); }

   void addSample(qint64 sample) {
    m_data[0] += sample;
   }

   void advance() {
    if (m_data.length() >= m_buckets) {
      m_data.resize(m_buckets - 1);
    }
    m_data.push_front(0);
   }

   qint64 average() const {
    qint64 sum = 0;
    for (auto x : m_data) {
      sum += x;
    }
    return m_data.isEmpty() ? 0 : sum / m_data.length();
   }

  private:
   const int m_buckets;
   QVector<qint64> m_data;
};

static QString bytesToString(qint64 bytes) {
  if (bytes < 1024) {
    return QString("%1b").arg(bytes);
  }

  if (bytes < 1024 * 1024) {
    return QString("%1Kb").arg(bytes / 1024);
  }

  if (bytes < 1024 * 1024 * 1024) {
    return QString("%1Mb").arg(bytes / (1024 * 1024));
  }

  return QString("%1Gb").arg(bytes / (1024 * 1024 * 1024));
}

struct CliOptions {
  uint16_t port = 0;
  QHostAddress addr = QHostAddress::LocalHost;
  QString localSocketName;
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

  QCommandLineOption localOption({"l", "local"}, "Local socket name", "name");
  parser.addOption(localOption);

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
  if (parser.isSet(localOption)) {
    out.localSocketName = parser.value(localOption);
  }
  if (parser.isSet(verboseOption)) {
    out.verbose = true;
  }
  return out;
};

static void startVerboseCLI(const Socks5* socks5) {
  static QList<Event> s_events;
  static QString s_lastStatus;
  static QTimer timer;

  // Run a boxcar average of the data trasnfered.
  constexpr const int max_counters = 8;
  static BoxcarAverage rx_average(max_counters);
  static BoxcarAverage tx_average(max_counters);

  auto cleanup = [&]() {
    // Update the boxcar average.
    rx_average.advance();
    tx_average.advance();
  
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

      QStringList addresses;
      for (const Event& event : s_events) {
        if (!event.m_newConnection.isEmpty() &&
            !addresses.contains(event.m_newConnection)) {
          addresses.append(event.m_newConnection);
        }
      }

      out << " [" << addresses.join(", ") << "]";
      out << " Up: " << bytesToString(tx_average.average()) << "/s";
      out << " Down: " << bytesToString(rx_average.average()) << "/s";
    }

    output.truncate(80);
    while (output.length() < 80) output.append(' ');
    QTextStream out(stdout);
    out << output << '\r';
    s_lastStatus = output;
  };
  QObject::connect(socks5, &Socks5::connectionsChanged,
                   [printStatus]() { printStatus(); });
  QObject::connect(
      socks5, &Socks5::incomingConnection,
      [printStatus](Socks5Connection* conn) {
        s_events.append(Event{conn->clientName(),
                              QDateTime::currentMSecsSinceEpoch()});
        printStatus();
      });

  QObject::connect(
      socks5, &Socks5::dataSentReceived,
      [](qint64 sent, qint64 received) {
        tx_average.addSample(sent);
        rx_average.addSample(received);
      });

  QObject::connect(&timer, &QTimer::timeout, [printStatus, cleanup]() {
    cleanup();
    printStatus();
  });
  timer.start(1000);

  // Install a message handler that plays nice with the verbose output.
  // Clears the current line - prints the log message - reprints the status.
  qInstallMessageHandler(
    [](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
      QTextStream out(stdout);
      out << QString(80, ' ') << '\r';
      out << msg << "\r\n";
      out << s_lastStatus << '\r';
    });
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

  Socks5* socks5;
  if (!config.localSocketName.isEmpty()) {
    QLocalServer* server = new QLocalServer();
    socks5 = new Socks5(server);
    if (server->listen(config.localSocketName)) {
      qDebug() << "Starting on local socket" << server->fullServerName();
    } else if ((server->serverError() == QAbstractSocket::AddressInUseError) &&
               QLocalServer::removeServer(config.localSocketName) &&
               server->listen(config.localSocketName)) {
      qDebug() << "(Re)starting on local socket" << server->fullServerName();
    } else {
      qWarning() << "Unable to listen to the local socket"
                 << config.localSocketName;
      qWarning() << "Listen failed:" << server->errorString();
      return 1;
    }
  } else {
    QTcpServer* server = new QTcpServer();
    socks5 = new Socks5(server);
    if (server->listen(config.addr, config.port)) {
      qDebug() << "Starting on port" << config.port;
    } else {
      qWarning() << "Unable to listen to the proxy port" << config.port;
      return 1;
    }
  }

  if (config.verbose) {
    startVerboseCLI(socks5);
  }

  QObject::connect(socks5, &Socks5::incomingConnection,
                   [](Socks5Connection* conn) {
                     qDebug() << "Connection from peer" << conn->clientName();
                   });

#ifdef __linux__
  new LinuxBypass(socks5);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  new WindowsBypass(socks5);
#endif

  return app.exec();
}
