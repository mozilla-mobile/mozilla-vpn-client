/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QLocalServer>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QString>
#include <QTcpServer>
#include <QTimer>

#include "socks5.h"
#include "sockslogger.h"

#ifdef __linux__
#  include "linuxbypass.h"
#endif
#if defined(PROXY_OS_WIN)
#  include "windowsbypass.h"
#  include "winsvcthread.h"
#endif

struct CliOptions {
  uint16_t port = 0;
  QHostAddress addr = QHostAddress::LocalHost;
  QString localSocketName;
  QString username = {};
  QString password = {};
  bool verbose = false;
  bool logfile = false;
#if defined(PROXY_OS_WIN)
  bool service = false;
#endif
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

#if defined(PROXY_OS_WIN)
  QCommandLineOption localOption({"n", "pipe"}, "SOCKS proxy over named pipe",
                                 "name");
#else
  QCommandLineOption localOption({"-n", "unix"} "SOCKS proxy over UNIX socket",
                                 "path");
#endif
  parser.addOption(localOption);

  QCommandLineOption logfileOption({"l", "logfile"}, "Save logs to file");
  parser.addOption(logfileOption);

  QCommandLineOption verboseOption({"v", "verbose"}, "Verbose");
  parser.addOption(verboseOption);

#if defined(PROXY_OS_WIN)
  QCommandLineOption serviceOption({"s", "service"}, "Windows service mode");
  parser.addOption(serviceOption);
#endif
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
  if (parser.isSet(logfileOption)) {
    out.logfile = true;
  }
  if (parser.isSet(verboseOption)) {
    out.verbose = true;
  }
#if defined(PROXY_OS_WIN)
  if (parser.isSet(serviceOption)) {
    out.service = true;
    // Enforce logging when started as a service.
    out.logfile = true;
  }
#endif
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

  auto *logger = new SocksLogger(&app);
  logger->setVerbose(config.verbose);
  if (config.logfile) {
    auto location = QStandardPaths::AppLocalDataLocation;
    QDir logdir(QStandardPaths::writableLocation(location));
    logger->setLogfile(logdir.filePath("socksproxy.log"));
  }

#if defined(PROXY_OS_WIN)
  if (config.service) {
    WinSvcThread* svc = new WinSvcThread("Mozilla VPN Proxy");
    svc->start();
  }
#endif

  Socks5* socks5;
  if (!config.localSocketName.isEmpty()) {
    QLocalServer* server = new QLocalServer(&app);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, server,
                     &QLocalServer::close);

    socks5 = new Socks5(server);
    server->setSocketOptions(QLocalServer::WorldAccessOption);
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
    QTcpServer* server = new QTcpServer(&app);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, server,
                     &QTcpServer::close);

    socks5 = new Socks5(server);
    if (server->listen(config.addr, config.port)) {
      qDebug() << "Starting on port" << server->serverPort();
    } else {
      qWarning() << "Unable to listen to the proxy port" << config.port;
      return 1;
    }
  }
  QObject::connect(socks5, &Socks5::incomingConnection, logger,
                   &SocksLogger::incomingConnection);

#ifdef __linux__
  new LinuxBypass(socks5);
#elif defined(PROXY_OS_WIN)
  new WindowsBypass(socks5);
#endif

  return app.exec();
}
