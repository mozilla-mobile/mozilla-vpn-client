/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLocalServer>
#include <QRandomGenerator>
#include <QString>
#include <QTcpServer>
#include <QTimer>

#include "socks5.h"
#include "verboselogger.h"

#ifdef __linux__
#  include "linuxbypass.h"
#endif
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  include "windowsbypass.h"
#endif

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
    new VerboseLogger(socks5);
  }

#ifdef __linux__
  new LinuxBypass(socks5);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  new WindowsBypass(socks5);
#endif

  return app.exec();
}
